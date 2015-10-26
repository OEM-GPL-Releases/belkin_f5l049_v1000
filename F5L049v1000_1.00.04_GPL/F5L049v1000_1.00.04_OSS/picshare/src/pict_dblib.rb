#!/usr/bin/ruby
# 
# pict_dblib.rb - LocalDB is made, it analyzed, and it search it. 
# 
# CONFIDENTIAL Copyright (C) 2008 - 2009 silex technology, Inc.
#

require 'find'
require 'pict_commonlib.rb'
require 'fileutils'
require 'syslog'
require 'pict_code'

########## DB_Module(Method) 
#module DB_Method
class DB_Main

	include MimeType
	attr_reader :upload_num

	BACKUP_EXT = ".db.back"
	LIST_EXT = ".list"
	DB_EXT = ".db"
	TMP_EXT = ".tmp"
	
	def initialize()
		@jobno = ""
		@jobdir = ""
		@size = ""
		@order = 0
		@dbname = "" 
		@dbback = "" 
		@listname = ""
		@tmp = ""
		@jobdir_split_size = 0
		@upload_num = 0
		@wp = DB_Write.new
		@rec = DB_GetRecord.new
		@db_sh = DB_Serch.new
		@list_sh = DB_Serch.new
	end

	def fin_db(tmp, name, order)
		return false if !::File.exist?(tmp)
		line = ""
		if order == $ORDER_OLD
			File::open("#{name}2.tmp", "w") { |w|
				if ::File.exist?(name)
					File::open(name, "r") { |r|
						while line = r.gets
							w.write(line)
						end
					}
				end
				if ::File.exist?(tmp)
					File::open(tmp, "r") { |r|
						while line = r.gets
							w.write(line)
						end
					}
				end
			}
		else # Newest Upload. or none order.
			File::open("#{name}2.tmp", "w") { |w|
				if ::File.exist?(tmp)
					File::open(tmp, "r") { |r|
						while line = r.gets
							w.write(line)
						end
					}
				end
				if ::File.exist?(name)
					File::open(name, "r") { |r|
						while line = r.gets
							w.write(line)
						end
					}
				end
			}
		end
		return false if !::File.exist?("#{name}2.tmp")
		FileUtils.mv("#{name}2.tmp", name)
		File.delete(tmp) if ::File.exist?(tmp) 
		return true
	end
	
	def file_ex(path)
		return false if !::File.exist?(path)
		ret = false
		if path[0..(@jobdir.length-1)] == @jobdir
			Dir.chdir(@jobdir) { |f|
				ret = true if ::File.exist?(path[@jobdir.length..path.length])
			}
		end
		return ret
	end

	def new_uploadlist
		@tmp = "%s%s" % [@listname, TMP_EXT]
		File.delete(@tmp) if ::File.exist?(@tmp)
		File::open(@tmp, "w"){} if !::File.exist?(@tmp)
		file_buf = []
		ret = true
		@wp.value_set(@tmp)
		Find.find(@jobdir) { |file|
			if @size > 0
				next if @size < File.size(file)
			end
			next if File::basename(file)[0..0] == "."
			next if !File.file?(file)
			next if !(mime = get_mimetype(file))
			if !@wp.write(file, File.size(file).to_s, File.mtime(file).to_s)
				ret = false
				break
			end
		}
		FileUtils.mv(@tmp, @listname)
		return ret
	end

	def renew_uploadlist
		@tmp = "%s%s" % [@listname, TMP_EXT]
		File.delete(@tmp) if ::File.exist?(@tmp)
		File::open(@tmp, "w"){ } if !::File.exist?(@tmp)
		file_buf = []
		
		return false if !::File.exist?(@dbname) or !::File.exist?(@jobdir)
		@db_sh.value_set(@dbname, @jobdir)
		return false if !::File.exist?(@listname) or !::File.exist?(@jobdir)
		@list_sh.value_set(@listname, @jobdir)
		@wp.value_set(@tmp)

		if File.size(@listname) != 0
			@rec.value_set(@listname)

			while @rec.get_record
				next if File::basename(@rec.path)[0..0] == "."
				next if @db_sh.serch_record(@rec.path, @rec.size, @rec.date) # Serch DB!
				next if !file_ex(@rec.path)
				if @size > 0
					next if @size < File.size(@rec.path)
				end
				next if (@rec.size.to_i != File.size(@rec.path)) or (@rec.date != File.mtime(@rec.path).to_s)
				if !@wp.write(@rec.path, @rec.size, @rec.date)
					return false
				end
			end
			FileUtils.mv(@tmp, @listname)
			File::open(@tmp, "w"){ }
		end

		# Add new image data.
		Find.find(@jobdir){ |file|
			next if !File.file?(file)
			next if File::basename(file)[0..0] == "."
			if @size > 0
				next if @size < File.size(file)
			end
			next if !(mime = get_mimetype(file))
			next if @list_sh.serch_record(file, File.size(file), File.mtime(file))
			next if @db_sh.serch_record(file, File.size(file), File.mtime(file))
			if !@wp.write(file, File.size(file), File.mtime(file))
				return false
			end
		}
		fin_db(@tmp, @listname, @order)
		return true
	end

	def check_db
		@tmp = "%s%s" % [@dbname, TMP_EXT]
		File.delete(@tmp) if ::File.exist?(@tmp)
		File::open(@tmp, "w"){} if !::File.exist?(@tmp)
		File::open(@listname, "w"){} if !::File.exist?(@listname)
		File::open(@dbback, "w"){} if File.size(@dbback) == 0 # New Job!
		if !::File.exist?(@dbname) # New Job!
			FileUtils.copy_file(@dbback, @dbname)
		else # Renew Job!
			FileUtils.copy_file(@dbback, @dbname) if !FileUtils.cmp(@dbname, @dbback)
			@wp.value_set(@tmp)
			@rec.value_set(@dbname)
			return false if !::File.exist?(@dbname)

			while @rec.get_record()
				next if File::basename(@rec.path)[0..0] == "."
				next if !::File.exist?(@rec.path)
				next if (@rec.size.to_i != File.size(@rec.path)) or (@rec.date != File.mtime(@rec.path).to_s)
				if !@wp.write(@rec.path, @rec.size, @rec.date)
					return false
				end
			end
			FileUtils.copy_file(@tmp, @dbback)
			FileUtils.mv(@tmp, @dbname)
		end
		return true
	end
	
	def value_set(jobno, dbdir, jobdir, size, order)
		@jobno = jobno
		@jobdir = jobdir
		@size = size
		@order = order
		@dbname = "%s%s%s" % [dbdir, @jobno, DB_EXT]
		@dbback = "%s%s%s" % [dbdir, @jobno, BACKUP_EXT]
		@listname = "%s%s%s" % [dbdir, @jobno, LIST_EXT]
		@tmp = ""
		@jobdir_split_size = (@jobdir.split("/").length)
		@upload_num = 0
	end
	
	def get_upload_num
		if ::File.exist?(@listname)
			File::open(@listname, "r") { |f|
				while f.gets
					@upload_num += 1
				end
			}
		end
	end

	def make_uploadlist_main(jobno, dbdir, jobdir, size, order)
		chret = 0
		ret = false
		method = ""
		value_set(jobno, dbdir, jobdir, size, order)
		if check_db
			if File.size(@dbname) == 0 and File.size(@listname) == 0
				method = "new_uploadlist"
				ret = true if new_uploadlist
			else
				method = "renew_uploadlist"
				ret = true if renew_uploadlist
			end
			get_upload_num
		else
			chret = 1
		end
		if !ret
			Syslog.open("picshare(R)") { |s|
				s.log(Syslog::LOG_ERR, "m_upl_M:%s, %s, %s, %d, %d, %d", jobno, dbdir, jobdir, size, order, chret)
			}
		end
		return ret
	end
	
end # End of Class

class DB_Write
	attr_accessor :filename

	public
	def initialize()
		@filename = ""
		@err_code = $NONE_ERR
		@backup = ""
		@writebuf = ""
	end

	def value_set(filename, backup=nil)
		@filename = filename
		@err_code = $NONE_ERR
		@backup = backup if backup
	end
	
	def write(path, size, date)
		ret = false
		@writebuf = "\"%s\",\"%d\",\"%s\"\n" % [path, size, date.to_s]
		if ::File.exist?(@filename)
			# File Write!
			File::open(@filename, 'a+') { |f|
				if f.flock(File::LOCK_EX) == 0
					f.write(@writebuf)
					f.flock(File::LOCK_UN)
					ret = true
				end
			}
		end
		return ret
	end
	
	def backup(path, size, date)
		ret = false
		@writebuf = "\"%s\",\"%d\",\"%s\"\n" % [path, size, date.to_s]
		if ::File.exist?(@backup)
			# File Write!
			File::open(@backup, 'a+') { |f|
				if f.flock(File::LOCK_EX) == 0
					f.write(@writebuf)
					f.flock(File::LOCK_UN)
					ret = true
				end
			}
		end
		return ret
	end

end # end of class

class DB_Serch
	include MimeType
	attr_accessor :filename
	attr_reader :maxindex, :jobdirpath
	private
		@file

	public
		def initialize()
			@filename = ""
			@jobdirpath = ""
			@ret = false
		end # end of init metnod
		
		def value_set(filename, jobdirpath)
			@filename = filename
			@jobdirpath = jobdirpath
			@ret = false
		end

		def serch_record(path, size, date)
			@ret = false
			line = ""
			if ::File.exist?(@filename)
				File::open(@filename, "r") { |f|
					while line = f.gets
						tmp = line.chomp.split("\"")
						next if tmp.size != 6
						next if tmp[3] != size.to_s
						next if tmp[5] != date.to_s
						next if tmp[1] != path
						@ret = true
						break
					end
				}
			end
			return @ret
		end

end #=> end of class DB_Serch

class DB_GetRecord

	attr_accessor :filename
	attr_accessor :path, :index, :date, :size
	attr_reader :maxindex
	private
		@file

	public
		def initialize()
			@filename = ""
			@ret = false
			@offset = 0
		end # end of init metnod
		
		def value_set(filename)
			@filename = filename
			@offset = 0
			@line = ""
		end

		def get_record
			@ret = false
			File::open(@filename, "r") { |f|
				f.seek(@offset, IO::SEEK_SET)
				while line = f.gets
					tmp = line.chomp.split("\"")
					next if tmp.size != 6
					@path = tmp[1]
					@size = tmp[3]
					@date = tmp[5]
					@ret = true
					break
				end
				@offset = f.tell
			}
			return @ret
		end #=> method get_record end

end #=> end of class DB_Read
