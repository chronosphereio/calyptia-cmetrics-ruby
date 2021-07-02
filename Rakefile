# frozen_string_literal: true

require "bundler/gem_tasks"
require "rake/testtask"
require "rake_compiler_dock"
require "rake/clean"

Rake::TestTask.new(:test) do |t|
  t.libs << "test"
  t.libs << "lib"
  t.test_files = FileList["test/**/*_test.rb"]
end

task default: [:compile, :test]

task test: [:compile]

require 'rake/extensiontask'

spec = eval(File.read("cmetrics-ruby.gemspec"))

Rake::ExtensionTask.new('cmetrics', spec) do |ext|
  ext.ext_dir = 'ext/cmetrics'
  ext.cross_compile = true
  ext.lib_dir = File.join(*['lib', 'cmetrics', ENV['FAT_DIR']].compact)
  # cross_platform names are of MRI's platform name
  ext.cross_platform = ['x86-mingw32', 'x64-mingw32']
end

task :clean do
  FileUtils.rm_f File.join(File.dirname(__FILE__), "ext", "cmetrics", "lib*.a")
  FileUtils.rm_rf File.join(File.dirname(__FILE__), "ext", "cmetrics", "cmetrics")
  FileUtils.rm_rf File.join(File.dirname(__FILE__), "ext", "cmetrics", "monkey")
  FileUtils.rm_f File.join(File.dirname(__FILE__), Dir.glob(".*.installed"))
  FileUtils.rm_rf File.join(File.dirname(__FILE__), "ports")
  FileUtils.rm_rf File.join(File.dirname(__FILE__), "tmp")
end
