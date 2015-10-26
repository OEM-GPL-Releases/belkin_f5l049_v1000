#!/usr/bin/ruby
# 
# pict_jobexe.rb - PictureSharing Job executes script. 
# 
# CONFIDENTIAL Copyright (C) 2008 - 2009 silex technology, Inc.
#

require 'fileutils'
require 'syslog'
require 'pict_code'
require 'pict_confserchlib'
require 'pict_commonlib'
require 'pict_joblist'
require 'pict_logmake'
require 'pict_dblib'
require 'pict_flickrlib'
require 'pict_picasalib'

	class PicShareJobExe
		include ProxySet

		JOBLIST = "/tmp/picshare/job.list"
		JOBLISTTMP = "/tmp/picshare/job.list.tmp"
		JOBLIST_LOCK = "/var/lock/picshare/job.list.lock"

		def initialize()
			@job = JobConf_Serch.new
			@job_w = JobConf_Write.new
			@user = UserConf_Serch.new
			@user_w = UserConf_Write.new
			@log = PICTLog.new
			@db = DB_Main.new
			@dbwrite = DB_Write.new
			@listwrite = DB_Write.new
			@rec = DB_GetRecord.new
			@joblist = PicShareJobList.new
			@flickr = Flickr.new
			@picasa = Picasa.new
			buf_clear
			time_value_set
		end
		
		def buf_clear()
			@jobno = ""
			@dbdir = ""
			@jobdir = ""
			@detail = "None"
			@code = $NONE_ERR
			@date = 0
			@pict = nil
			@size = $UPLOAD_SIZE
			@listname = ""
			@dbname = ""
			@db_backupname = ""
			@tmp = ""
			@time = ""
			@bnum = 0
			@anum = 0
		end

		def time_value_set
			@cn = {}
			@sc = {}
			min = 0
			if @joblist.get_exec_jobno("3")
				@sc = {'option' => 2, 'time' => "-1"} # upload now
			else
				now = Time.now.getlocal
				if now.min >= 30 and now.min <= 40
					min = 30
				elsif now.min >= 0 and now.min <= 10
					min = 0
				end
				time = "%02d:%02d" % [now.hour, min]
				case now.hour
				when 0, 3, 6, 9, 12, 15, 18, 21
					if min == 0
						@sc['option'] = 1 # anytime
					else
						@sc['option'] = 0 # select time
					end
				else # select time
					@sc['option'] = 0
				end
				@sc['time'] = time # upload scdule
			end
		end

		def hdd?
			if !::File.exist?(@job.hdd)
				@code = $HDD_NONE
				@date = 0
			else
				return true
			end
			return false
		end

		def db?
			@dbdir = "%s%s%s/" % [@job.hdd, $DB_DIR, @jobno]
			# DB Directory Check.
			if !::File.exist?("#{@dbdir}#{@jobno}#{$DB_NAME}#{$DB_BACKUP_NAME}")
				FileUtils.makedirs(@dbdir) if !::File.exist?(@dbdir)
				@code = $FILE_ERR
				@date = 0
				@detail = "None Local DB."
			else
				return true
			end

			return false
		end
		
		def jobdir?
			@jobdir = "%s%s" % [@job.mntp, @job.dir_path]
			# Job Directory Check.
			if !::File.exist?(@jobdir)
				@code = $FILE_ERR
				@date = 0
				@detail = "None Job Directory."
				return false
			end
			return true
		end
		
		def user?
			# Get User Info.
			return false if @jobno == ""
			if !@user.user_serch(@job.user_no, @job.web_service)
				@code = $SYSTEM_ERR
				@date = 0
				return false
			end
			return true
		end
		
		def push_log(cn)
			if @jobno != "" and @dbdir != ""
				str = "%s%s.xml" % [@dbdir, @jobno] # Log set!
				@log.buf_clear(str, @jobno, cn)
				if !@log.write_log(@jobno, @job.web_service, @user.user, File.basename(@job.dir_path), @code, @detail)
					@code = $FILE_ERR # Aother error.
					@date = Time.now.to_i
				else
					@code = @log.code
					@date = @log.date
				end
			end
		end
		
		def job?
			return fail if !@job.job_serch(@jobno) # jobno none.
			return true
		end
		
		def flickr_info
			@pict = @flickr
			@pict.first($FLICKR_API, $FLICKR_SECRET)
			@pict.jobs_clear(@user.pass, @job.access, @job.tag)

			ret = @pict.get_userinfo
			# ScreenName & Order Check!
			if @pict.user_name != @user.user and ret == true
				# User name Renew?
				if !@user_w.renew(@job.web_service, @job.user_no, @pict.user_name, @user.pass)
					@code = $SYSTEM_ERR
					@detail = "Userconf renew error!(name)"
				end
				user_name = @pict.user_name
			end

			# Upload Stoped?
			if @job.order_stop != 0 and @job.order == 0
				@code = $LIMIT_ERR
				@detail = "No Order."
				return false
			end

			# Upload Stop?
			if @pict.err_code == $LIMIT_ERR
				# Upload order none. Upload stop.
				if @job.order == 0
					# Job Conf Renew!
					if !@job_w.renew(@job.job_no, nil, nil, 1)
						@code = $SYSTEM_ERR
						@detail = "Jobconf renew error!(upload stop!)"
					end
				end
			 end

			if @code == $NONE_ERR and @pict.err_code != $NONE_ERR
				@code = @pict.err_code
				@detail = @pict.detail
			end

			if @code == $NONE_ERR or (@code == $LIMIT_ERR and @job.order != 0)
				return true
			else
				return false
			end
		end
		
		def picasa_info
			@pict = @picasa
			@pict.jobs_clear(@user.user, @user.pass, @job.id)

			if @pict.auth
				if @pict.get_userinfo
					if @pict.get_albuminfo
						if @pict.err_code != $LIMIT_ERR or @pict.err_code != $ALBUMLIMIT_ERR
							if @pict.album_title != @job.title or @job.access != @pict.album_access
								# Album title or Album Access change
								if !@job_w.renew(@job.job_no, "#{@pict.album_title}", @pict.album_access, 0)
									@code = $SYSTEM_ERR
									@detail = "Jobconf renew error!(albumtitle & album access)"
								end
							end
						end
					end
				end
			end
			if @code == $NONE_ERR and @pict.err_code != $NONE_ERR
				@code = @pict.err_code
				@detail = @pict.detail
			end

			if @code == $NONE_ERR
				return true
			else
				return false
			end
		end

		def make_db
			@ret = false
			@ret = @db.make_uploadlist_main(@jobno, @dbdir, @jobdir, @size, @job.order)
			@bnum = @db.upload_num
			if !@ret
				@code = $FILE_ERR
				@detail = "Make db fail."
			end
			return @ret
		end

		def pictbuf_clear()
			# value
			@listname = "%s%s%s" % [@dbdir, @jobno, $LIST_NAME]
			@dbname = "%s%s%s" % [@dbdir, @jobno, $DB_NAME]
			@db_backupname = "%s%s%s%s" % [@dbdir, @jobno, $DB_NAME, $DB_BACKUP_NAME]
			@tmp = "#{@listname}.tmp"
			@time = $RETRY_NUM
			@bnum = @db.upload_num
			@anum = 0
			# class
			File::open(@tmp, "w") { } if !::File.exist?(@tmp)
			@dbwrite.value_set(@dbname, @db_backupname)
			@listwrite.value_set(@tmp)
			@rec.value_set(@listname)
		end
		
		def upload
			return false if @code != $NONE_ERR

			pictbuf_clear
			if !::File.exist?(@listname)
				@code = $FILE_ERR
				@detail = "DB(upload list) None."
			else
				cn = 0

				while @rec.get_record
					if cn > 20 # Garbage collection
						cn = 0
						GC.start
					end
					next if !::File.exist?(@rec.path)
					next if File.size(@rec.path) != @rec.size.to_i or File.mtime(@rec.path).to_s != @rec.date
					
					cn = cn + 1
					if @code == $LIMIT_ERR
						if !@listwrite.write(@rec.path, @rec.size, @rec.date)
							@code = $FILE_ERR
							@detail = "ERR: Write(FailList)"
							break
						end

					else
						if !@dbwrite.write(@rec.path, @rec.size, @rec.date)
							@code = $FILE_ERR
							@detail = "ERR: Write(LocalDB)"
							break
						end

						# Upload!
						if (@pict.photo_upload(@rec.path))
							@dbwrite.backup(@rec.path, @rec.size, @rec.date)
						else # UploadERROR
							@code = @pict.err_code
							@detail = @pict.detail
							if !@listwrite.write(@rec.path, @rec.size, @rec.date)
								@code = $FILE_ERR
								@detail = "ERR: Write(FailList)"
								break
							end

							if @code == $TIMEOUT_ERR
								if (@time -= 1) == 0
									@detail = "ERR:Upload(Timeout)" 
									break
								end
							elsif @code != $UPLOAD_ERR and @code != $LIMIT_ERR
								@detail = "ERR:UpLoad(code:%04x)" % [@code] if @detail == ""
								break
							end
						end
					end

					# set job status.
					@anum += 1
					@joblist.set_jobstatus(2, @jobno, @code, @date, @bnum, @anum)
				end
				FileUtils.mv(@tmp, @listname) if ::File.exist?(@tmp)
			end

			if @job.order == 0 and @code == $LIMIT_ERR and @job.web_service == $FLICKR_CODE
				# Job Conf Renew!
				if !@job_w.renew(@jobno, nil, nil, 1)
					@code = $SYSTEM_ERR
					@detail = "Jobconf renew error!(upload stop!)"
				end
			end

			if File.size(@listname) != 0 and @code == $NONE_ERR
				@code = $UPLOAD_ERR
				@detail = "ERR: Upload(upload err)"
			end
		end
		
		def scdule?(time=nil)
			if @sc['option'] == 2 # upload now
				if @joblist.get_exec_jobno("3") # upload now
					return true
				end
			else # upload scdule
				@cn = @job.get_optiontime
				if time
					@sc = time if time['option'] > 0
				end
				if @sc['option'] == 1 and @cn['option'] == 1
					return true # scdule anytime
				elsif @sc['time'] == @cn['time']
					return true # scdule select
				end
			end
			return false
		end
		
		def lock(pid, filename="")
			if filename == ""
				if @jobno != ""
					filename = @jobno
				else
					return true
				end
			end
			if ::File.exist?("#{$LOCK_DIR}#{filename}")
				return false
			else
				File::open("#{$LOCK_DIR}#{filename}", "w") { |f|
					f.puts(pid)
				}
			end
			return true
		end
		
		def unlock(pid, filename="")
			if filename == ""
				if @jobno != ""
					filename = @jobno
				else
					return true
				end
			end
			ret = false
			if ::File.exist?("#{$LOCK_DIR}#{filename}")
				File::open("#{$LOCK_DIR}#{filename}", "r") { |f|
					if l = f.gets
						ret = true if pid == l.chomp!.to_i
					end
				}
				File.delete("#{$LOCK_DIR}#{filename}") if ret
			end
		end
		
		def dellock(filename="")
			if filename == ""
				if @jobno != ""
					filename = @jobno
				else
					return true
				end
			end
			if ::File.exist?("#{$LOCK_DIR}#{filename}")
				File.delete("#{$LOCK_DIR}#{filename}")
			end
			return true
		end

		def alljob_unlock(pid)
			jobno_list = @job.get_confjobno
			return true if jobno_list.size == 0
			l = ""
			ret = false
			jobno_list.each { |job|
				if ::File.exist?("#{$LOCK_DIR}#{job}")
					ret = false
					File::open("#{$LOCK_DIR}#{job}", "r") { |f|
						if l = f.gets
							ret = true if pid == l.chomp!.to_i
						end
					}
					File.delete("#{$LOCK_DIR}#{job}") if ret
				end
			}
			return true
		end

		def store_flashrom(path)
			`/sbin/sxromconf -c STORE_CFG -o #{path}`
			`/sbin/sxromconf -c FLUSHCACHE`
		end

		def exe(pid)
			begin
				pid = Process.pid
				kill_flg = false
				if !lock(pid, "runnig")
					Syslog.open("picshare(R)") { |s|
						s.log(Syslog::LOG_ERR, "Job duplicate error.")
					}
					@joblist = nil
					return false
				end
				sc = nil
				jobopt = ["1","3"] # scdule upload , upload now
				js = {}
				str = ""

				while (1)
					begin
						# Trap
						Signal.trap("SIGUSR2") do
							raise SignalException, "SIGUSR2"
						end
						Signal.trap("TERM") do
							raise SignalException, "SIGTERM"
						end

						buf_clear
						js = {}

						if ::File.exist?(JOBLIST)
							if !(sc = @joblist.get_joblist_status)
								break
							end
						end
						@joblist.make_joblist(1, @sc['option'], @sc['time'], pid, @joblist.flg)

						if !scdule?(sc)
							break if !(@jobno = @joblist.get_exec_jobno("3"))
						else
							break if !(@jobno = @joblist.get_exec_jobno(jobopt))
						end

						next if !job?
						next if @job.active != $JOB_ACTIVE
						next if !hdd?

						next if !(js = @log.get_jobstatus(@jobno))
						if !lock(pid) # lock fail ?
							Syslog.open("picshare(R)") { |s|
								s.log(Syslog::LOG_ERR, "Job lock error.")
							}
							dellock()
							next if !lock(pid)
						end
						
						@joblist.set_jobstatus(2, @jobno, js['code'], js['date'], @bnum, @anum) # set job status.

						if user?
							if db? and jobdir?
								proxy_read() # Proxy set!

								# webserver connect!
								if @job.web_service == $FLICKR_CODE
									@ret = flickr_info
								elsif @job.web_service == $PICASA_CODE
									@ret = picasa_info
								end

								if @ret
									if make_db # make db
										GC.start
										upload # picture upload
									end
								end
							end
						end
					rescue SignalException => e # Job Cancel.
						# puts(" > SignalException! %s" % e.message)
						kill_flg = true if e.message == "SIGTERM" # Job all cancel!
						@code = $KILL_ERR
						@detail = "Job Cancel."
						push_log(0)
						
					rescue Exception => e # Catch! Exception.
						# puts(" > Exception! %s" % e.message)
						@code = $SYSTEM_ERR if @code == $NONE_ERR
						@detail = "JobException: %s" % [e.message]
						push_log(1)
						Syslog.open("picshare(R)") { |s|
							s.log(Syslog::LOG_ERR, "Exception!(%x:%s)", @code, @detail)
						}

					ensure
						# puts(" > Job Ensure!")
						if @code != $KILL_ERR
							if (@code == $NONE_ERR and @bnum != 0) or (@code != $NONE_ERR)
								push_log(1)
							else
								if js.size != 0
									@code = js['code']
									@date = js['date']
								end
							end
						end

						@joblist.set_jobstatus(0, @jobno, @code, @date, 0, 0) # set job status.
						unlock(pid) if @jobno != ""
						GC.start if !kill_flg
					end
					break if kill_flg
				end

			rescue Exception => e # Catch! Exception.
				# puts(" > Exception! %s" % e.message)
				@detail = "%s" % [$!.to_s]
				Syslog.open("picshare(R)") { |s|
					s.log(Syslog::LOG_ERR, "Exception!(-1:%s)", @detail)
				}

			ensure
				# puts(" > Ensure!")
				if @joblist
					@joblist.make_joblist(0, -1,"-1", pid, '3') # flg = 3 => end.
				end
				store_flashrom("/etc/sysconfig")

				alljob_unlock(pid)
				unlock(pid, "runnig")
			end
		end

	end		# end of class 'PicShareJobExe'
