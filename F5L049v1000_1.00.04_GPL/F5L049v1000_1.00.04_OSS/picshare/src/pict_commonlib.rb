#!/usr/bin/ruby
# 
# pict_commonlib.rb - MIME Type is distinguished, HTTP Request is made, and HTTP Proxy is set. 
# 
# CONFIDENTIAL Copyright (C) 2008 - 2009 silex technology, Inc.
#

require 'socket'
require 'net/http'
require 'pict_code.rb'

module MimeType

	def get_mimetype(file_path)
		mime = File::extname(file_path)
		case mime.chomp.downcase
		when '.jpg', '.jpeg'
			return 'image/jpeg'
		when '.bmp'
			return 'image/bmp'
		when '.gif'
			return 'image/gif'
		when '.png'
			return 'image/png'
		else
			return false
		end
	end # get_mimetype end

end # module MimeType end

module Net

	class HTTPGenericRequest
		def send_request_with_body_stream(sock, ver, path, f)
			unless content_length() or chunked?
				raise ArgumentError,
				"Content-Length not given and Transfer-Encoding is not `chunked'"
			end
			supply_default_content_type
			write_header sock, ver, path
			if chunked?
				while s = f.read(1024)
					sock.write(sprintf("%x\r\n", s.length) << s << "\r\n")
				end
				sock.write "0\r\n\r\n"
			else
				buf=String.new
				f.collect{ |fd|
					while buf = fd.read(1024)
						sock.write(buf)
					end
				}
			end
		end
	end

end # module Net end

module CreateRequest

	def create_request(method, uri = "/", header_tab=nil, len=0, data_tab=nil)
		case method
			when "GET"
				req = Net::HTTP::Get.new(uri)
			when "PUT"
				req = Net::HTTP::Put.new(uri)
			when "POST"
				req = Net::HTTP::Post.new(uri)
			else
				return nil
		end

		header_tab.each { |key, value|
			req.add_field(key, value) 
		}
		if data_tab
			body_stream = []
			data_tab.collect{ |data|
				body_stream << (data.is_a?(IO) ? data : StringIO.new(data))
			}
			req.body_stream = body_stream
		end

		req.content_length = len
		return req
	end

end # module CreateRequest end

module ConfCheck

	def str_cut(str, maxsize)
		ret = ""
		str.split(//).each { |c|
			break if (ret.size + c.size) > maxsize
			ret << c
		}
		return ret
	end

end # module ConfCheck end

module ProxySet
	PROXY_ADDR = 'HTTP_PROXY_ADDR'
	PROXY_PORT = 'HTTP_PROXY_PORT'

	def proxy_read
		line = ""
		conf = ""
		
		# lock
		`touch "#{$CONF_LOCK_FILE}" > /dev/null` if !::File.exist?($CONF_LOCK_FILE)
		fd = ::File.open($CONF_LOCK_FILE, "r")
		if fd.flock(File::LOCK_SH) != 0
			fd.close
			return false
		end

		fp = ::File.open($SYS_CONF, 'r')
		while line = fp.gets
			conf = line.split(/[=]/)[0]
			case conf
			when PROXY_ADDR
				$DPROXY_ADDR = line.split(/[=]/)[1].chomp
			when PROXY_PORT
				if (line.split(/[=]/)[1].chomp) == ""
					$DPROXY_PORT = 0
				else
					$DPROXY_PORT = line.split(/[=]/)[1].chomp.to_i
				end
			end
		end
		fp.close

		# unlock
		fd.flock(File::LOCK_UN)
		fd.close

		if $DPROXY_ADDR != "" or $DPROXY_PORT != 0
			$DPROXY = true
		else
			$DPROXY = false
		end
		
		return true
	end
end # module Proxy end
