#!/usr/bin/ruby
# 
# pict_flickrlib.rb - Photo is sent to Flickr, HTTP Response is analyzed. 
# 
# CONFIDENTIAL Copyright (C) 2008 - 2009 silex technology, Inc.
#

require 'digest/md5'
require 'cgi'
require "stringio"
require 'net/http'
require 'rexml/document'
require 'pict_commonlib'
require 'pict_code'

class Flickr

	include Net
	include CreateRequest
	include MimeType
	include ConfCheck

	attr_reader :upload_size, :err_code, :detail, :user_name
	
	def initialize()
		@upload_datas = Array.new
		@upload_requestdata_tab = Array.new
		@boundary = "---------------------------#{rand(10000000000)}"
		@upload_header_tab = {"Content-Type" => "multipart/form-data; boundary=" + @boundary}
		@upload_data_footer = "\r\n--#{@boundary}--\r\n"
		@upload_uri = "/services/upload/"
		@httpbody = ""
		@err_num = 0
		@err_code = $NONE_ERR
		@api_key = ""
		@secret = ""
		@upload_param = {}
		@token = ""
		@proxy = {}
		@user_name = ""
		@pro = 0
		@upload_size = 0
		@upload_param = {}
		@hp = nil
	end

	def first(api_key, secret)
		@api_key = api_key
		@secret = secret
		@upload_param = {'api_key' => @api_key, 'auth_token' => "" }
	end
	
	def jobs_clear(token, access=nil, tags=nil)
		@upload_datas = []
		@upload_requestdata_tab = []
		@httpbody = ""
		@err_num = 0
		@err_code = $NONE_ERR
		@detail = "NONE"
		@token = token
		@upload_param['auth_token'] = token
		@proxy = { :addr => $DPROXY_ADDR, :port => $DPROXY_PORT }
		@user_name = ""
		@pro = 0
		@upload_size = 0
		if access
			if access[0] == $ACCESS_PUBLIC[0] # Puclic
				@upload_param['is_public'] = 1
				@upload_param['is_friend'] = 0
				@upload_param['is_family'] = 0
			else # Private
				@upload_param['is_public'] = 0
				@upload_param['is_family'] = 1 if access[1] == $ACCESS_SUB_ON[0]
				@upload_param['is_friend'] = 1 if access[2] == $ACCESS_SUB_ON[0]
			end
		end
		@upload_param['tags'] = tags if tags
		@upload_param['api_sig'] = make_sig(@secret, @upload_param)
		if $DPROXY
			@hp = Net::HTTP::Proxy(@proxy[:addr], @proxy[:port]).new('www.flickr.com',80)
		else
			@hp = Net::HTTP.new('www.flickr.com',80)
		end
	end

	def make_url_path(url_param)
		url_path = (url_param.collect{ |key, value| "#{key}=#{CGI::escape(value)}"}).join('&')
		return ('?' + url_path)
	end
 
	def make_sig(secret, param)
		return Digest::MD5.hexdigest(secret + param.sort.flatten.join).to_s
	end

	def get_userinfo
		begin
			@httpbody = ""
			@ret = false
			sigNature = make_sig(@secret, {'api_key' => @api_key, 
				'method' => 'flickr.people.getUploadStatus', 'auth_token' => @token})
			url_path = '/services/rest/' + make_url_path({ :method => 'flickr.people.getUploadStatus',
				:api_key => @api_key, :auth_token=> @token, :api_sig => sigNature })

			if $DPROXY
				http = Net::HTTP::Proxy(@proxy[:addr], @proxy[:port]).new('api.flickr.com',80)
			else
				http = Net::HTTP.new('api.flickr.com',80)
			end
			http.open_timeout = $NETWORK_TIMEOUT
			http.read_timeout = $NETWORK_TIMEOUT
			http.start { |hp|
				@httpbody = hp.get(url_path)
			}
			@ret = response_analyze(@httpbody, $FLICKR_GETUSERINFO)
			return @ret

		rescue Timeout::Error
			@err_code = $TIMEOUT_ERR
			@detail = "Timeout!(Userinfo)"
			@ret = false
			return @ret
		rescue 
			@err_code = $TIMEOUT_ERR
			@detail = "Raise(Userinfo:%s)" % [$!.to_s]
			@ret = false
			return @ret
		end
	end

	def make_requestdata(filename, mime)
		f = File::basename(filename, File.extname(filename))
		@upload_param.each {|k,v|
			@upload_datas <<  "--" + @boundary + "\r\n"
			@upload_datas << "Content-Disposition: form-data; name=\"#{k}\"\r\n"
			@upload_datas << "\r\n"
			@upload_datas << v
			@upload_datas << "\r\n"
		}
		@upload_datas <<  "--" + @boundary + "\r\n"
		@upload_datas << "Content-Disposition: form-data; name=\"photo\"; filename=\"#{f}\"\r\nContent-Type: #{mime}\r\n"
		@upload_datas << "\r\n"
		return @upload_datas.join
	end
	
	def photo_upload_new
		@upload_requestdata_tab.clear
		@upload_datas.clear
		@len = 0
		@ret = false
		@httpbody = ""
		@req = ""
		@fp = ""
		@err_code = $NONE_ERR
	end

	def photo_upload(file)
		begin
			photo_upload_new()
			@upload_requestdata_tab << make_requestdata(File::basename(file), get_mimetype(file))
			@len = (File.size(file)) + @upload_requestdata_tab[0].length + @upload_data_footer.length
			@fp = File.open(file, 'rb')
			@upload_requestdata_tab << @fp
			@upload_requestdata_tab << @upload_data_footer
	  		@req = create_request('POST', @upload_uri, @upload_header_tab, @len, @upload_requestdata_tab)
			@hp.open_timeout = $NETWORK_TIMEOUT
			@hp.read_timeout = $NETWORK_TIMEOUT
			@hp.start { |http|
				@httpbody = http.request(@req)
			}
			@ret = response_analyze(@httpbody, $FLICKR_ADDPHOTO)
			return @ret
		rescue Timeout::Error
			@err_code = $TIMEOUT_ERR
			@detail = "Timeout!(Upload)"
			@ret = false
			return @ret
		rescue 
			@err_code = $TIMEOUT_ERR
			@detail = "Raise(Upload:%s)" % [$!.to_s]
			@ret = false
			return @ret
		end
	end
	
	def response_analyze(response, action)
		@err_code = $NONE_ERR
		case response.code.to_i
		when 200 # Response OK
			xml_root = REXML::Document.new(response.body).root
			xml_root.attributes.each { |state, value| 
				xml_root.elements.each { |tag| 
					# Error Response.
					if (state != 'stat' or value != 'ok') and tag.name == 'err'
						@detail = tag.attributes['msg'].to_s
						case tag.attributes['code'].to_i
						when 6 # Limit!
							@err_code = $LIMIT_ERR
						when 2,3,4,5
							@err_code = $UPLOAD_ERR
						when 96,97,98,99,100
							@err_code = $AUTH_ERR
						else
							@err_code = $WEBSERVER_ERR
						end
						return false
					end

					if state=='stat' and value == 'ok'
						case action
						when $FLICKR_GETUSERINFO # get_userinfo
							tag.elements.each{ |user, val|
								case user.name
								when 'username'
									@user_name = str_cut(user.text, $USER_CONF_NAME_LEN)

								when 'bandwidth'
									if user.attributes['unlimited'] == '0'
										maxbytes = user.attributes['maxbytes'].to_i
										usedbytes = user.attributes['usedbytes'].to_i
										if maxbytes < usedbytes
											@err_code = $LIMIT_ERR
											@detail = "Get Userinfo"
										end
									end

						 		when 'filesize'
						 			@upload_size = user.attributes['maxbytes'].to_i
						 		end
						 	}
						 	return true if @upload_size != 0 and (@err_code == $NONE_ERR or @err_code == $LIMIT_ERR)

						when $FLICKR_ADDPHOTO # photo_upload
							return true if tag.name == 'photoid'
						end
					end
				}
			}
		else
			@err_code = $WEBSERVER_ERR
			@detail = "HTTP Response Code(%d)" % [response.code.to_i]
		end
		return false
	end
end
