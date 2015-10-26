#!/usr/bin/ruby
# 
# pict_jobexe.rb - PictureSharing Daemon script 
# 
# CONFIDENTIAL Copyright (C) 2008 - 2009 silex technology, Inc.
#

# Picture Sharing
require 'fileutils'
require 'syslog'
require 'pict_jobexe'

# WAN Backup
#require 's3job.rb'
#require 's3config'
#require 's3rest'
#require 's3pack'
#require 's3log'

module PicShare

	# Picture Sharing
	PIDFILE = "/var/lock/picshare/picshare.pid"
	OLD_PIDFILE = "/var/lock/picshare/joblist"
	JOBLIST_DIR = "/tmp/picshare/"
	PICT_CONF = "/etc/silex/picshare.conf"
	
	# WAN Backup
#	PPIDFILE = "/var/run/wanbkp.pid"
#	CPIDFILE = "/var/run/wanbkc.pid"

	def self.lock(pid)
		return false if ::File.exist?(PIDFILE)
		File::open(PIDFILE, "w") { |f| f.puts(pid) }
		File::open(OLD_PIDFILE, "w") { |f| f.puts(pid) }
		return true
	end
	
	def self.unlock(pid)
		if ::File.exist?(PIDFILE)
			if pid == 0 # shutdown now!
				del = ture
			else
				del = false
				File::open(PIDFILE, "r") {|f|
					if line = f.gets
						del = true if ((line.chomp!).to_i) == pid
					end
				}
			end
			if del
				File.delete(PIDFILE)
				File.delete(OLD_PIDFILE)
			end
		end
	end
	
	def self.shutdown
		if ::File.exist?(PIDFILE)
			val = 0
			File::open(PIDFILE, "r") { |f|
				if line = f.gets
					val = ((line.chomp!).to_i)
				end
			}
			if val != 0
				begin
					Process.kill("TERM", val)
				rescue
					# nop
				end
			end
		end
	end
	
	def self.set_pictconf
		# Error Check!
		if !::File.exist?(PICT_CONF)
			Syslog.open("picshare(D)") { |s|
				s.log(Syslog::LOG_ERR, "No PICT_CONF(/etc/silex/picshare.conf")
			}
			return false
		end
		File::open(PICT_CONF, "r") { |f|
			line = ""
			buf = []
			set = ""
			while line = f.gets
				buf = []
				buf = line.split('=')
				str = buf[1].chomp
				case buf[0]
				when 'MNTP'
					$MNTP = str.to_s

				when 'SYS_CONF'
					$SYS_CONF = str.to_s

				when 'CONF_LOCK_FILE'
					$CONF_LOCK_FILE = str.to_s

				when 'LOCK_DIR'
					$LOCK_DIR = str.to_s

				when 'DB_DIR'
					$DB_DIR = str.to_s

				when 'JOB_CONF'
					$JOB_CONF = str.to_s
				
				when 'PICASA_USER_CONF'
					$PICASA_USER_CONF = str.to_s
				
				when 'FLICKR_USER_CONF'
					$FLICKR_USER_CONF = str.to_s

				when 'UPLOAD_SIZE'
					$UPLOAD_SIZE = str.to_i
				
				when 'FLICKR_API'
					$FLICKR_API = str.to_s

				when 'FLICKR_SECRET'
					$FLICKR_SECRET = str.to_s
				
				when 'NETWORK_TIMEOUT'
					$NETWORK_TIMEOUT = str.to_i

				when 'RETRY_NUM'
					$RETRY_NUM = str.to_i

				when 'LOG_DETAIL_LEN'
					$LOG_DETAIL_LEN = str.to_i
				
				when 'LOG_COUNT'
					$LOG_COUNT = str.to_i
				end

			end
		}
		return true
	end

	def self.main
		begin 
			$PICT_PID = 0
			#$wan_pid = 0
			pid = Process.pid

			# WAN Backup Lock!
			#::File.open(PPIDFILE, "w") { |pf| pf.puts(Process.pid) }

			# Picture Sharing Lock!
			if !lock(pid)
				Syslog.open("picshare(D)") { |s|
					s.log(Syslog::LOG_ERR, "Duplicate error.")
				}
				self.shutdown
				lock(pid)
			end

			# WAN backup (Backup now)
			#Signal.trap("USR1") do	# run right away
			#	self.wan_run(1)
			#end
			
			# Picture Sharing (Upload now)
			Signal.trap("USR2") do
				self.pict_run(pid) # Job None.
			end

			# Process(daemon) shutdown
			Signal.trap("TERM") do
				# WAN Backup
				#if $wan_pid > 0
				#	begin
				#		Process.kill("TERM", $wan_pid)
				#	rescue
				#		# nop
				#	end
				#end

				# Picture Sharing
				if $PICT_PID > 0
					begin
						Process.kill(15, $PICT_PID)
					rescue
						# nop
					end
				end
				exit
			end
			
			# WAN Backup initialize.
			#config = WANBackup::Config.new()
			#config.load()
			#order = WANBackup::Order.new
			#order.create(config, false)	# create -> save (locked)

			# Picture Sharing initialize.
			raise if !set_pictconf # Value set.

			# main loop
			loop do
				now = Time.now.getlocal
				if now.sec == 0 then break end
				sleep(1)
			end

			loop do
				sleep(60)
				now = Time.now.getlocal # current time
				if now.min == 0 || now.min == 30 # each 30 min
					self.pict_run(pid) # Job None.
			#		self.wan_run(0)
				end
			end
		ensure
			unlock(pid)
			Syslog.open("picshare(D)") { |s|
				s.log(Syslog::LOG_ERR, "The end.")
			}
		end
	end


	#def self.wan_run(mode)
	#	puts("wan run")
	#	pid = 0
	#	wan_exec = true
	#
	#	if $wan_pid > 0
	#		begin
	#			if Process.waitpid($wan_pid, Process::WNOHANG) == nil
	#				wan_exec = false
	#			end
	#		rescue Errno::ECHILD	# no child process
	#			wan_exec = true
	#		end
	#	end
	#
	#	if wan_exec
	#		pid = fork do
	#			if mode == 0
	#				WANBackup.scheduled_main
	#			else
	#				WANBackup.manual_main
	#			end
	#		end
	#		Process.detach(pid)
	#		$wan_pid = pid
	#	end
	#	return pid
	#end

	def self.pict_run(pid)
		pict_exec = true

		FileUtils.makedirs(JOBLIST_DIR) if !File::exist?(JOBLIST_DIR)
		
		if $PICT_PID > 0
			begin
				if Process.waitpid($PICT_PID, Process::WNOHANG) == nil
					pict_exec = false
				end
			rescue  Errno::ECHILD
				pict_exec = true
			end
		end

		if pict_exec
			puts("")
			puts("Upload! Job Start!!!")
			puts("------")
			pict = PicShareJobExe.new
			if pict.scdule?
				 p = fork do
					pict.exe(p) # Job None.
				end
					#printf("pict_pid = %d\n", p)
					Process.detach(p)
					$PICT_PID = p
			end
		end

	end		# end of method 'main'
	
end		# end of module

