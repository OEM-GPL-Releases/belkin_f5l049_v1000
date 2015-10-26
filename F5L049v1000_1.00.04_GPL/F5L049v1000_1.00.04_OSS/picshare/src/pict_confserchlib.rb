#!/usr/bin/ruby
# 
# pict_confserchlib.rb - Search and renew of cnfigure file. 
# 
# CONFIDENTIAL Copyright (C) 2008 - 2009 silex technology, Inc.
#

require 'find'
require 'fileutils'
require 'pict_code'
require 'pict_commonlib'

class UserConf_Serch
	attr_reader :user_no, :user, :pass, :index

	public
	def initialize()
		@user_no = ""
		@usr = ""
		@pass = ""
		@index = 0
	end # end of init metnod

	def user_serch(no, web)
		return false if no == ""
		ret = false
		filename = ""
		method = "r"
		method = "w" if !::File.exist?($CONF_LOCK_FILE)

		File::open($CONF_LOCK_FILE, method){ |fd|
			# lock!
			break if fd.flock(File::LOCK_SH) != 0
				# conffile check!
				if web.to_i == $FLICKR_CODE
					filename = $FLICKR_USER_CONF
				elsif web.to_i == $PICASA_CODE
					filename = $PICASA_USER_CONF
				else
					break
				end
				break if !::File.exist?(filename)
				
				# Conf File Open!
				File::open(filename, "r"){ |cn|
					line = ""
					serch_userno = "#{$USER_CONF_NO}=#{no}"
				
					while line = cn.gets
						if line[0..1] == $CONF_SPLIT
							@index = line[3..5].to_i
							next
						end
						next if line.chomp != serch_userno # No Hit.
						@user_no = no
						while line[0..1] != $CONF_SPLIT
							line = (cn.gets).chomp
							case line.split(/[=]/)[0]
							when $USER_CONF_PASS
								@pass = line[3, line.size]
							when $USER_CONF_NAME
								@user = line[3, line.size]
							end
						end
						ret = true
						break
					end
				} # conffile close
		} # unlock conffile

		return ret
	end		# end of method 'user_serch'

end		# end of class 'UserConf_Serch'

class JobConf_Serch
	attr_reader :job_no, :user_no, :dir_path, :mntp, :hdd
	attr_reader :order, :web_service, :active, :access, :index
	##picasa
	attr_reader :id, :title
	##flickr
	attr_reader :tag, :order_stop

	private
	@conf_file

	public
	def initialize()
		@job_no = ""
		@user_no = ""
		@dir_path = ""
		@mntp = ""
		@hdd = ""
		@order = 0
		@web_service = 0
		@index = 0
		@order_stop = 0
		@access = $ACCESS_PRIVATE
		@conf_file = nil
	end # end of init metnod

	def job_serch(no)
		return false if no == ""
		method = "r"
		ret = false
		
		# lock!
		method = "w" if !::File.exist?($CONF_LOCK_FILE)
		File::open($CONF_LOCK_FILE, method){ |fd|
		break if fd.flock(File::LOCK_SH) != 0
		
			# Conf File Open!
			break if !::File.exist?($JOB_CONF)
			File::open($JOB_CONF, "r"){ |cn|
				serch_jobno = "#{$JOB_CONF_NO}=#{no}"
				line = ""
			
				while line = cn.gets
					if line[0..1] == $CONF_SPLIT
						@index = line[3..5].to_i
						next
					end
					next unless line.chomp == serch_jobno
					@job_no = no
					while line[0..1] != $CONF_SPLIT
						line = (cn.gets).chomp
						case line.split(/[=]/)[0]
						when $JOB_CONF_NO
							@job_no = line[3, line.size]
						when $USER_CONF_NO
							@user_no = line[3, line.size]
						when $JOB_CONF_PATH
							@dir_path = line[3, line.size]
						when $JOB_CONF_ORDER
							@order = line[3, line.size].to_i
						when $JOB_CONF_WEB
							@web_service = line[3, line.size].to_i
						when $JOB_CONF_ACTIVE
							@active = line[3, line.size].to_i
						when $JOB_CONF_ORDERSTOP
							@order_stop = line[3, line.size].to_i

						#picasa only
						when $JOB_CONF_PICASA_AID
							@id = line[3, line.size]
						when $JOB_CONF_PICASA_ATI
							@title = line[3, line.size]
						when $JOB_CONF_PICASA_AAC
							@access = line[3, line.size]
							
						#flickr only
						when $JOB_CONF_FLICKR_TAG
							@tag = line[3, line.size]
						when $JOB_CONF_FLICKR_PAC
							@access = line[3, line.size]
						else
							break
						end
					end
					ret = true
					buf = @dir_path.split('/')
					@mntp = "%s" % [$MNTP]
					@hdd = "%s%s/" % [$MNTP, buf[0]]
					break
				end
			} # jobconf close
		} # unlock jobconf

		return ret
	end		# end of method 'job_serch'
	
	def get_confjobno()
		fd = nil
		jobno_list = []
		# errcheck!
		if ::File.exist?($JOB_CONF)
			method = "r"
			method = "w" if !::File.exist?($CONF_LOCK_FILE)

			File::open($CONF_LOCK_FILE, method){ |fd|
				# lock!
				break if fd.flock(File::LOCK_SH) != 0

				# Conffile open
				File::open($JOB_CONF, "r"){ |cn|
					line = ""
					while line = cn.gets
						next if line[0..1] != $JOB_CONF_NO
						line.chomp!
						if line[3, line.size] != ""
							jobno_list<< line[3, line.size]
						end
					end
				} # jobconf close
			} # unlock jobconf
		end

		return jobno_list
	end		# end of method 'get_confjobno'

	def get_optiontime()
		return false if !::File.exist?($JOB_CONF)
		sc = { 'option' => $JOB_OPTION_UPNOW, 'time' => '-1' }
		line = ""
		ret = false
		method = "r"
		# lock!
		method = "w" if !::File.exist?($CONF_LOCK_FILE)

		File.open($CONF_LOCK_FILE, method){ |fd|
			# lock!
			break if fd.flock(File::LOCK_SH) != 0

			File::open($JOB_CONF, 'r'){ |fp|
				line = fp.gets.chomp
				ret = true
			} # jobconf close
		} # unlock jobconf

		# Schdule set?
		buf = line.split(/[,|=]/)
		if buf[0] == 'option'
			sc['option'] = buf[1].to_i
			sc['time'] = "%s" % [buf[3]]
		end

		if !ret
			return false
		else
			return sc
		end
	end		# end of method 'get_optiontime'

end		# end of class 'JobConf_Serch'

class JobConf_Write

	CONF_JOB_TMP = '/tmp/pictjob'

	public
#	def initialize()
#	end # end of init metnod
	
	def renew(job_no, album_title=nil, album_access=nil, order_stop=nil)
		return false if !::File.exist?($JOB_CONF)
		return false if album_access.to_i < 0 or album_access.to_i > 1
		method = "r"

		# lock!
		method = "w" if !::File.exist?($CONF_LOCK_FILE)

		File::open($CONF_LOCK_FILE, method){ |fd|
			# lock!
			break if fd.flock(File::LOCK_EX) != 0
			
			# open jobconf(original)
			File::open($JOB_CONF, 'r'){ |fp|
				# open jobconf(tmp)
				File::open(CONF_JOB_TMP, 'w'){ |wp|
					buf = ""
					flg = false
					line = ""
					cmd = ""
					serch_jobno = $JOB_CONF_NO + '=' + job_no

					while line = fp.gets
						if flg or line.chomp != serch_jobno
						 	wp.write(line)
						 	next
						 end

						# Hit!
						wp.write(line)
						while line[0..1] != $CONF_SPLIT
							line = fp.gets
							cmd = line.split(/[=]/)[0]
							case cmd
							# picasa only
							when $JOB_CONF_PICASA_ATI
								buf = "%s=%-.*s\n" % [cmd, $JOB_CONF_PICASA_ATI_LEN, album_title]
								wp.write(buf)
								buf = ""
							when $JOB_CONF_PICASA_AAC
								buf = "#{cmd}=#{album_access}\n"
								wp.write(buf)
								buf = ""
							# flickr only
							when $JOB_CONF_ORDERSTOP
								buf = "#{cmd}=#{order_stop}\n"
								wp.write(buf)
								buf = ""
							else
							 	wp.write(line)
							 	next
							end
						end
						flg = true
					end
				} # jobconf close(write)
			} # jobconf close(read only)

			FileUtils.mv(CONF_JOB_TMP, $JOB_CONF)
		} # unlock jobconf

		return true
	end		# end of method 'renew'

end		# end of method 'JobConf_Write'

########## User Conf Write!
class UserConf_Write

	CONF_USER_TMP = '/tmp/pictuser'

	public

	def renew(web, user_no, name, pass)
		ret = false
		if web.to_i == $FLICKR_CODE
			@filename = $FLICKR_USER_CONF
		elsif web.to_i == $PICASA_CODE
			@filename = $PICASA_USER_CONF
		else
			return ret
		end
		return ret if !::File.exist?(@filename)
		method = "r"
		mechod = "w" if !::File.exist?($CONF_LOCK_FILE)

		File::open($CONF_LOCK_FILE, method){ |fd|
			# lock!
			break if fd.flock(File::LOCK_EX) != 0

			com = ""
			buf = ""
			flg = false
			serch_userno = $USER_CONF_NO + '=' + user_no

			File::open(@filename, "r"){ |fp|
				File::open(CONF_USER_TMP, "w"){ |wp|
					while line = fp.gets
						if flg or line.chomp != serch_userno
							wp.write(line)
							next
						end

						# Hit!
						wp.write(line)
						while line[0..1] != $CONF_SPLIT
							line = fp.gets
							cmd = line.split(/[=]/)[0]
							case cmd
							when $USER_CONF_PASS
								buf = "%s=%-.*s\n" % [cmd, $USER_CONF_NAME_LEN, pass]
								# user name length == user passwd length.
								wp.write(buf)
								buf = ""
							when $USER_CONF_NAME
								buf = "%s=%-.*s\n" % [cmd, $USER_CONF_NAME_LEN, name]
								wp.write(buf)
								buf = ""
							else
								wp.write(line)
							end
						end
						flg = true
					end

				} # userconf close(write)
			} # userconf close(read only)
			
			ret = true
			FileUtils.mv(CONF_USER_TMP, @filename)
		} # unlock userconf

		return ret
	end

end #=> end of class
