#!/usr/bin/ruby
# 
# pict_joblist.rb - Joblist(temporary file) is made and search. 
# 
# CONFIDENTIAL Copyright (C) 2008 - 2009 silex technology, Inc.
#

require 'fileutils'
require 'pict_code'
require 'pict_confserchlib'
require 'pict_commonlib'

	class PicShareJobList

		JOBLIST = "/tmp/picshare/job.list"
		JOBLISTTMP = "/tmp/picshare/job.list.tmp"
		JOBLIST_LOCK = "/var/lock/picshare/job.list.lock"

		attr_reader :flg

		def initialize()
			@conf = JobConf_Serch.new
			@log = PICTLog.new
			@flg = 0
		end

		def get_jobinfo(jobno, option, dm, listfile=nil)
			jobno_list = []
			no = ""
			status = 0
			code = 0
			date = 0
			line = ""
			method = "r"
			method = "w" if !::File.exist?(JOBLIST_LOCK)

			File.open(JOBLIST_LOCK, method) { |ld|
				# lock!
				break if ld.flock(File::LOCK_SH) != 0

				if !listfile or !::File.exist?(listfile)
					# None Joblist
					jobno.each{ |job|
						jobno_list << "%d,%s" % [dm, job]
					}
				else
					# Joblist open
					File::open(listfile, "r"){ |fp|
						jobno.each{ |job|
							while line = fp.gets
								no = line.split(",")[1]
								next if no != job
								status = line.split(',')[0].to_i
								code = line.split(',')[2].hex
								date = line.split(',')[3].to_i
								jobno_list<< "%d,%s,%x,%d" % [status, job, code, date]
								break
							end
							jobno_list << "%d,%s" % [dm, job] if no != job
							fp.seek(IO::SEEK_SET)
						}
					} # joblist close
				end
			} # unlock joblist

			return jobno_list
		end		# end of method 'get_jobinfo'

		def get_exec_jobno(sc, jobno=nil)
			ret = false
			method = "r"
			method = "w" if !::File.exist?(JOBLIST_LOCK)

			File::open(JOBLIST_LOCK, method){ |ld|
				# lock!
				break if ld.flock(File::LOCK_SH) != 0
				# joblist none...
				break if !::File.exist?(JOBLIST)

				sc.collect{ |opt|
					# get exec jobno(1(wait) or 3(upload now))
					File::open(JOBLIST, "r"){ |fp|
						fp.gets # line 1(global otpion)
						while line = fp.gets
							buf = line.split(",")
							next if buf.size != 6
							if buf[0] == opt and buf[1].size == 36
								ret = buf[1]
								break
							end

							if jobno # Select jobno
								if ret != jobno
									ret = false
									next
								end
							end
							break if ret
						end
					} # joblist close

					break if ret
				}

			} # unlock joblist

			return ret
		end		# end of method 'get_exec_jobno'

		def get_joblist_status
			sc = {}
			method = "r"
			method = "w" if !::File.exist?(JOBLIST_LOCK)

			File::open(JOBLIST_LOCK, method){ |ld|
				# lock!
				break if ld.flock(File::LOCK_SH) != 0
				
				# get clobal option.
				File::open(JOBLIST, "r") { |fp|
					break if !(line = fp.gets)
					buf = (line.chomp!).split(",")
					if buf[0] == '0' or buf[0] == '1'
						sc['flg'] = buf[0]
						if buf[0] == '0' # no move
							sc['option'] = -1 
							sc['time'] = -1
						else # move
							sc['option'] = buf[1].to_i
							sc['time'] = buf[2]
							sc['pid'] = buf[3].to_i
						end
						@flg = buf[0]
					end

				} # joblist close
			} # joblist unlock!

			return sc
		end		# end of method 'get_joblist_status'

		def set_jobstatus(option, jobno, code, date, bnum, anum)
			line = ""
			method = "r"
			ret = false
			method = "w" if !::File.exist?(JOBLIST_LOCK)
			File::open(JOBLIST_LOCK, method){ |ld|
				# lock!
				break if ld.flock(File::LOCK_EX) != 0
				# joblist none...
				break if !::File.exist?(JOBLIST)

				File::open(JOBLIST, "r"){ |fp|
					File::open(JOBLISTTMP, "w"){ |wp|
						line = fp.gets
						wp.write(line) # line 1(global option)
						while line = fp.gets
							buf = line.split(",")
							if buf.size == 6
								if buf[1].size == 36 and jobno == buf[1]
									line = "%d,%s," % [option, jobno]
									line += "%x,%d," % [code, date]
									line += "%d,%d\n" % [bnum, anum]
									ret = true
								end
								wp.write(line)
							end
						end

						# ?? Error?(No jobno hit.)
						if !ret
							line = "%d,%s," % [option, jobno]
							line += "%x,%d," % [code, date]
							line += "%d,%d\n" % [bnum, anum]
							wp.write(line)
							ret = true
						end

					}# joblist(tmp) close
				} # joblist close

				# file rename("JOBLISTTMP"(renew joblist) -> "JOBLIST")
				FileUtils.mv(JOBLISTTMP, JOBLIST)
			} # joblist unlock!

			return ret
		end		#  end of method 'set_jobstatus'
		
		def make_joblist(status, option, time, pid, flg)
			dm = 0
			path = ""
			js = {}
			method = "r"
			if (jobno = @conf.get_confjobno) # get jobno
				if option == 2 # upload-now
					path = JOBLIST
				else # scdule
					dm = 1 if flg != '3' # uploading
					path = JOBLIST if flg == '1'
				end
				
				# make joblist!
				if (jobno_list = get_jobinfo(jobno, option, dm, path))
					method = "w" if !::File.exist?(JOBLIST_LOCK)
					File::open(JOBLIST_LOCK, method){ |ld|
						# lock!
						break if ld.flock(File::LOCK_EX) != 0

						# joblist write!
						File::open(JOBLISTTMP, "w"){ |fp|
							# global option
							line = "%d,%d,%s,%d\n" % [status, option, time, pid]
							fp.write(line)
							line = ""
							
							# job info
							jobno_list.each{ |v|
								if v.split(",").size == 4
									line = "%s,0,0\n" % [v] if v.split(",")[2] != $HDD_NONE
								else
									if (js = @log.get_jobstatus(v.split(",")[1]))
										if js['code'] != $HDD_NONE
											line = "%s,%x,%d,0,0\n" % [v, js['code'], js['date']]
										end
									end
								end
								fp.write(line) if line != ""
							}
						}

						# file rename("JOBLISTTMP"(renew joblist) -> "JOBLIST")
						FileUtils.mv(JOBLISTTMP, JOBLIST)
					} # unlock joblist
				end
			end
		end		# end of method 'make_jobstatus'

	end		# end of class 'PicShareJobList'
