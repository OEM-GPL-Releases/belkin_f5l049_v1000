#!/usr/bin/ruby
# 
# pict_picasalib.rb - Photo is sent to Picasa, HTTP Response is analyzed. 
# 
# CONFIDENTIAL Copyright (C) 2008 - 2009 silex technology, Inc.
#

require 'net/http'
require 'net/https'
require 'cgi'
require 'rexml/document'
require "stringio"
require 'pict_commonlib.rb'
require 'pict_code.rb'


class Picasa
	include Net
	include CreateRequest
	include MimeType
	include ConfCheck

	attr_reader :err_code, :detail, :album_title, :album_access

	def initialize()
		@accounttype = "GOOGLE"
		@service = "lh2"
		@source = "Picasasync-App-1"
		@boundary = "---------------------------BelkinHS#{rand(10000000000)}"
		@upload_header_tab = {"Content-Type" => "multipart/related; boundary=" + @boundary,
		"MIME-version:" => "1.0"}
		@upload_data_footer = "\r\n--#{@boundary}--\r\n"

		@upload_uri = ""
		@user_name = ""
		@password = ""
		@token = ""
		@err_code = $NONE_ERR
		@album_title = ""
		@album_access = $ACCESS_PRIVATE
		@upload_datas = Array.new
		@upload_albumid = ""
		@upload_requestdata_tab = Array.new
		@hp = nil
		@httpbody = ""
	end
	
	def jobs_clear(user_name, password, album_id)
		@user_name = user_name
		@password = password
		@upload_albumid = album_id
		@token = ""
		@err_code = $NONE_ERR
		@detail = "NONE"
		@album_title = ""
		@album_access = $ACCESS_PRIVATE
		@upload_datas = []
		@upload_uri = "/data/feed/api/user/#{CGI::escape(@user_name)}/albumid/#{@upload_albumid}"
		@upload_requestdata_tab = []
		@httpbody = ""
		if $DPROXY
			@hp = Net::HTTP::Proxy($DPROXY_ADDR, $DPROXY_PORT).new('picasaweb.google.com',80)
		else
			@hp = Net::HTTP.new('picasaweb.google.com',80)
		end
	end

	def auth
		begin
			@http_body = ""
			@ret = false
			@err_code = $NONE_ERR
			auth_param = { 
				"accountType" => @accounttype,
				"Email" => @user_name, "Passwd" => @password,
				"service" => @service, "source" => @source,
			}
			param = auth_param.collect{ |key, value| "#{key}=#{CGI::escape(value)}"}.join('&')
			if $DPROXY
				auth_http = Net::HTTP::Proxy($DPROXY_ADDR, $DPROXY_PORT).new('www.google.com',443)
			else
				auth_http = Net::HTTP.new('www.google.com',443)
			end
			auth_http.open_timeout = $NETWORK_TIMEOUT
			auth_http.read_timeout = $NETWORK_TIMEOUT
			auth_http.use_ssl = true
			auth_http.verify_mode = OpenSSL::SSL::VERIFY_NONE
			auth_http.start { |http|
				@httpbody = http.post('/accounts/ClientLogin', param)
			}
			if auth_analyze(@httpbody)
				@infoparam = { "Authorization" => "GoogleLogin auth=\"#{@token}\"" }
				@photoparam = {'Authorization' => "GoogleLogin auth=\"#{@token}\""}
				@upload_header_tab['Authorization'] = "GoogleLogin auth=\"#{@token}\""
				@ret = true
			end

			return @ret
		rescue Timeout::Error
			@err_code = $TIMEOUT_ERR
			@detail = "Timeout!(Auth)"
			@ret = false
			return @ret
		rescue 
			@err_code = $TIMEOUT_ERR
			@detail = "Raise(Auth:%s)" % [$!.to_s]
			@ret = false
			return @ret
		end
	end # auth end

	def get_userinfo
		begin
			@httpbody = ""
			@ret = false
			@err_code = $NONE_ERR
			uri = "/data/feed/api/user/#{CGI::escape(@user_name)}/contacts?kind=user"
			@hp.open_timeout = $NETWORK_TIMEOUT
			@hp.read_timeout = $NETWORK_TIMEOUT
			@hp.start { |http|
				@httpbody = http.get(uri, @infoparam)
			}
			@ret = userinfo_albuminfo_analyze(@httpbody, 1)

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
	end # get_userinfo end
	
	def get_albuminfo
		begin
			@httpbody = ""
			@ret = false
			@err_code = $NONE_ERR
			uri = "/data/feed/api/user/#{CGI::escape(@user_name)}/albumid/#{@upload_albumid}?kind=tag"
			@hp.open_timeout = $NETWORK_TIMEOUT
			@hp.read_timeout = $NETWORK_TIMEOUT
			@hp.start { |http|
				@httpbody = http.get(uri, @infoparam)
			}
			@ret = userinfo_albuminfo_analyze(@httpbody, 2)

			return @ret
		rescue Timeout::Error
			@err_code = $TIMEOUT_ERR
			@detail = "Timeout!(Albuminfo)"
			@ret = false
			return @ret
		rescue 
			@err_code = $TIMEOUT_ERR
			@detail = "Raise(Albuminfo:%s)" % [$!.to_s]
			@ret = false
			return @ret
		end
	end # get_userinfo end

	def make_requestdata(filename, mime)
		f = File::basename(filename, File.extname(filename))
		f.gsub!(/[&]/, '&amp;')
		f.gsub!(/[<]/, '&lt;')
		f.gsub!(/[>]/, '&gt;')
		f.gsub!(/["]/, '&quot;')
	
		@upload_datas.clear
		@upload_datas << "\r\n"
		@upload_datas << "--" + @boundary + "\r\n"
		@upload_datas << "Content-Type: application/atom+xml\r\n"
		@upload_datas << "\r\n"
		@upload_datas << "<?xml version='1.0' encoding='UTF-8'?>"
		@upload_datas << "<entry xmlns='http://www.w3.org/2005/Atom'>"
		@upload_datas << "<title>" + f + "</title>"
		@upload_datas << "<category scheme=\"http://schemas.google.com/g/2005#kind\" "
		@upload_datas << "term=\"http://schemas.google.com/photos/2007#photo\"/>"
		@upload_datas << "</entry>\r\n"
		@upload_datas << "\r\n"
		@upload_datas << "--" + @boundary + "\r\n"
		@upload_datas << "Content-Type: " + mime + "\r\n"
		@upload_datas << "\r\n"
		return @upload_datas.join
	end

	def photo_upload(file)
		begin
			@httpbody = ""
			@ret = false
			@req = ""
			@fp = ""
			@err_code = $NONE_ERR

			@upload_requestdata_tab.clear
			@len = 0
			@upload_requestdata_tab << make_requestdata(file, get_mimetype(file))
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
			@ret = photo_analyze(@httpbody)
			@fp.close if @fp.is_a?(IO)
			if !@ret and @err_code == $ALBUMLIMIT_ERR
				@err_code = $NONE_ERR
				get_albuminfo
				@ret = false
				if @err_code != $ALBUMLIMIT_ERR
					@err_code = $LIMIT_ERR
					@detail = "Picasa Limit!"
				end
			end

			return @ret
		rescue Timeout::Error
			@err_code = $TIMEOUT_ERR
			@detail = "Timeout!(Upload:%s)" % [$!.to_s]
			@ret = false
			@fp.close if @fp.is_a?(IO)
			return @ret
		rescue 
			@err_code = $TIMEOUT_ERR
			@detail = "Raise(Upload:%s)" % [$!.to_s]
			@ret = false
			@fp.close if @fp.is_a?(IO)
			return @ret
		end
	end # photo_upload end

	### Response Data Analyze.
	def auth_analyze(response)
		case response.code.to_i
		when 200 #=> Response OK
			@token = response.body.split(' ')[2].gsub(/Auth=/){}
			if @token == ""
				@err_code = $AUTH_ERR
			else
				return true
			end
		when 400, 401, 403
			@err_code = $AUTH_ERR
			@detail = "HTTP Response Code(%d)" % [response.code.to_i]
		else # 200, 302, 404, 409, 500
			@err_code = $WEBSERVER_ERR
			@detail = "HTTP Response Code(%d)" % [response.code.to_i]
		end
		return false
	 end

	# Analyze : add_album & get_userinfo
	def userinfo_albuminfo_analyze(response, action)
		case response.code.to_i
		when 200 # Sucess! (Response OK)
			totallimit = usedbyte = 0 if action == 1 # get_userinfo
			albumusednum = albumfreenum = 0 if action == 2
			xml_root = REXML::Document.new(response.body).root
			xml_root.elements.each { |tag|
				case action
				when 1 # get_userinfo
					if tag.prefix == 'gphoto'
						case tag.name
						when 'quotalimit'
							totallimit = tag.text.to_i
						when 'quotacurrent'
							usedbyte = tag.text.to_i
						end
					end
					
				when 2 # get_albuminfo
					case tag.name
					when 'title'
						@album_title = str_cut(tag.text, $JOB_CONF_PICASA_ATI_LEN)
					when 'rights'
						if tag.text == 'public'
							@album_access = $ACCESS_PUBLIC
						else
							@album_access = $ACCESS_PRIVATE
						end
					when 'numphotosremaining'
						if tag.prefix == 'gphoto'
							albumfreenum = tag.text.to_i
						end
					end
				end
			}
			case action
			when 1
				if totallimit and usedbyte
					return true if totallimit > usedbyte
					@err_code = $LIMIT_ERR
					@detail = "Total limit over."
				end
			when 2
				if albumfreenum
					return true if albumfreenum != 0
					@err_code = $ALBUMLIMIT_ERR
					@detail = "Album limit over."
				end
			end
		when 400, 401, 403
			@err_code = $AUTH_ERR
			@detail = "HTTP Response Code(%d)" % [response.code.to_i]
		when 404
			@err_code = $ALBUM_ERR
			@detail = "HTTP Response Code(%d)" % [response.code.to_i]
		else # 201, 302, 409, 500
			@err_code = $WEBSERVER_ERR
			@detail = "HTTP Response Code(%d)" % [response.code.to_i]
		end
		return false
	end # common_analyze end

	def photo_analyze(response)
		case response.code.to_i
		when 201 # Sucess!
			xml_root = REXML::Document.new(response.body).root
			xml_root.elements.each { |tag|
				return true if tag.prefix == 'gphoto' and tag.name == 'id'
			}
		when 403
			@err_code = $ALBUMLIMIT_ERR
			@detail = "HTTP Response Code(%d)" % [response.code.to_i]
		when 200, 400, 409 # Upload Error!
			@err_code = $UPLOAD_ERR
			@detail = "HTTP Response Code(%d)" % [response.code.to_i]
		when 401
			@err_code = $AUTH_ERR
			@detail = response.body
		else # 404, 500, 304
			@err_code = $WEBSERVER_ERR
			@detail = "HTTP Response Code(%d)" % [response.code.to_i]
		end
		return false
	end

end # Class end
