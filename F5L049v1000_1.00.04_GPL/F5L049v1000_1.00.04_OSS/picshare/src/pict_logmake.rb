#!/usr/bin/ruby
# 
# pict_logmake.rb - Logfile is made, and analyzed. 
# 
# CONFIDENTIAL Copyright (C) 2008 - 2009 silex technology, Inc.
#

require 'fileutils'
require 'pict_code'
require 'pict_confserchlib'

class PICTLog

	attr_reader :filename
	attr_reader :code
	attr_reader :date
	
	private
		@split_buf

	public
		def initialize()
			@check = []
			@split_buf = []
			@lock = ""
			@xml = ""
			@code = 0
			@date = 0
			@filename = ""
			@count = 0
			@del = 0
			@method = "r"
			@ld = nil
			@line = ""
			@file = nil
		end
		
		def buf_clear(filename, job_no, cn)
			@check.clear
			@split_buf.clear
			@lock = ""
			@xml = ""
			@code = 0
			@date = 0
			@filename = filename
			if $LOG_COUNT > 0 and cn > 0
				@count = count(job_no)
				if @count >= $LOG_COUNT
					@del = (@count+1) - $LOG_COUNT
					delete_log(job_no)
				end
			elsif cn == -1
				File.delete(filename) if ::File.exist?(filename)
			else
				@count = 0
				@del = 0
			end
			@ld = nil
			@line = ""
			@file = nil
		end

		def count(job_no)
			cn = 0
			# lock!
			@lock = "#{$LOCK_DIR}#{job_no}.log"
			if !::File.exist?(@lock)
				@method = "w" # log lock file make
			end
			File.open(@lock, @method) { |ld|
				break if ld.flock(File::LOCK_SH) != 0
			
				if File::exist?(@filename)
					File::open(@filename, "r") { |f|
						while @line = f.gets
							cn += 1 if @line[0..3] == "<LOG"
						end
					}
				end

				ld.flock(File::LOCK_UN)
			}

			return cn
		end
	
		def delete_log(job_no)
			@line = ""
			flg = 0
			c = 0
			ret = false

			# lock!
			@lock = "#{$LOCK_DIR}#{job_no}.log"
			if !::File.exist?(@lock)
				@method = "w" # log lock file make
			end
			File::open(@lock, @method) { |ld|
				break if ld.flock(File::LOCK_EX) != 0
				File::open(@filename, "r"){ |file|
					File::open("#{@filename}.tmp", "w") { |f|
						while @line = file.gets
							if @line[0..3] == "<LOG" and flg == 0
								c += 1
								flg = 1 if c == @del
								next
							end
							f.write(@line)
						end
					}
				}
				if flg == 1
					FileUtils.mv("#{@filename}.tmp", @filename) if ::File.exist?("#{@filename}.tmp")
					ret = true
				end
				ld.flock(File::LOCK_UN)
			}
			return ret
		end
	
		def write_log(job_no, web, username, path, code, detail="None")
			ret = false
			@check = [username, path, detail]
			@check.collect{|buf|
				buf = " " if buf == "" or !buf
				buf.gsub!(/[&]/, '&amp;')
				buf.gsub!(/[<]/, '&lt;')
				buf.gsub!(/[>]/, '&gt;')
				buf.gsub!(/["]/, '&quot;')
			}
			@split_buf = path.split("/")
			@date = Time.now.to_i
			@code = code
			@xml = "<LOG date=\"%d\" web=\"%u\" " % [@date, web]
			@xml += "user=\"%-.*s\" " % [$USER_CONF_NAME_LEN, username]
			@xml += "path=\"%-.*s/\">" % [$JOB_CONF_PATH_END_LEN, @split_buf.pop]
			@xml += "<code>%0*x</code>" % [$ERR_CODE_LEN, @code]
			@xml += "<detail>%-.*s</detail></LOG>\r\n" % [$LOG_DETAIL_LEN, detail]
			
			# lock!
			@lock = "#{$LOCK_DIR}#{job_no}.log"
			if !::File.exist?(@lock)
				@method = "w" # log lock file make
			end
			File::open(@lock, @method) { |ld|
				break if ld.flock(File::LOCK_EX) != 0

				# add log!
				if ::File.exist?(@filename) # renew log
					File::open(@filename, "r+"){ |file|
						file.seek(-'</PICTLOG>'.length, IO::SEEK_END)
						file.write(@xml)
						file.write('</PICTLOG>')
					}
				else # new logfile
					File::open(@filename, "w+") { |file|
						file.puts('<?xml version="1.0" encoding="UTF-8"?>')
						file.puts('<PICTLOG job_no="' + job_no + '">')
						file.write(@xml)
						file.write('</PICTLOG>')
					}
				end
				ld.flock(File::LOCK_UN)
				ret = true
			}

			@filename = ""
			return ret
		end
	
		def get_logstatus(filename, job_no, web, userno, path)
			status = {'code' => $LOG_NONE, 'date' => 0 }
			sp = nil
			ep = nil

			if ::File.exist?(filename)
				@lock = "#{$LOCK_DIR}#{job_no}.log"
				if !::File.exist?(@lock)
					@method = "w" # log lock file make
				end
				File::open(@lock, @method){ |ld|
					if ld.flock(File::LOCK_SH) != 0
						status['code'] = $LOG_NONE # No Log.
						break
					end

					if File.size(filename) < $LOG_LINE_SIZE
						s_size = File.size(filename)
					else
						s_size = $LOG_LINE_SIZE
					end
					File.open(filename, "r"){ |file|
						file.seek(-s_size, IO::SEEK_END)
						@xml = file.read(s_size)
						if (sp = @xml.rindex("<code>"))
							if (ep = @xml[sp..@xml.size].index("</code>"))
								ep = sp + ep
								if sp > 0 and ep > sp
									status['code'] = @xml[(sp+6)..(ep+sp+6-1)].hex
									if (sp = @xml.rindex("<LOG date="))
										if (ep = @xml[(sp+11)..s_size].index("\""))
											status['date'] = @xml[(sp+11)..(ep+sp+11-1)].to_i
										end
									end
								end
							end
						end
					}
					@xml = ""
				}

				if !sp or !ep # Analyze error!
					user = UserConf_Serch.new
					user.user_serch(userno, web)
					buf_clear(filename, job_no, -1)
					write_log(job_no, web, user.user, path, $SYSTEM_ERR, "Log Analyze Error!")
					status['code'] = @code
					status['date'] = @date
				end
			end
			return status
		end
	
		def get_jobstatus(jobno)
			job = JobConf_Serch.new
			return false if !job.job_serch(jobno) # jobno none.
			status = { 'date' => 0, 'code' => $LOG_NONE }
			# Status get!
			dir_buf = job.dir_path.split('/')

				# HDD Check
				if ::File.exist?(job.hdd) # /mnt/shared/[Drive name]/
					# DB Directory(log file) Check.
					log_path = "%s%s%s/%s%s" % [job.hdd, $DB_DIR, jobno, jobno, $LOG_NAME]
					if ::File.exist?(log_path)
						status = get_logstatus(log_path, jobno, job.web_service, job.user_no, job.dir_path)
					else
						status['code'] = $LOG_NONE # No Log.
					end

				else
					status['code'] = $HDD_NONE # No HDD.
				end
			
			return status
		end
	
end		# end of class
