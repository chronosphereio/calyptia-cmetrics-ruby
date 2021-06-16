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

task default: [:"build:cmetrics", :compile, :test]

task test: [:"build:cmetrics", :compile]

require 'rake/extensiontask'

class BuildCMetrics
  require "mini_portile2"
  require "fileutils"

  def initialize(version=nil)
    @version = if version
                version
              else
                "master".freeze
              end
    @recipe = MiniPortileCMake.new("cmetrics", @version)
    @checkpoint = ".#{@recipe.name}-#{@recipe.version}.installed"
    @recipe.files << {
      url: "file://#{File.dirname(__FILE__)}/ext/#{@recipe.name}-#{@recipe.version}.zip",
      sha256sum: "1c8703a926fb93dc90abb63da8f1e3caaa0a2243892434c7ec5c6fba27e53d8f",
    }
  end

  def build
    unless File.exist?(@checkpoint)
      @recipe.cook
      lib_path = File.join(File.dirname(__FILE__), "ports/#{@recipe.host}/cmetrics/#{@version}/lib/libcmetrics.a")
      include_path = File.join(File.dirname(__FILE__), "ports/#{@recipe.host}/cmetrics/#{@version}/include/")
      FileUtils.cp(lib_path, File.join(File.dirname(__FILE__), "ext", "cmetrics", "libcmetrics.a"))
      FileUtils.cp_r(Dir.glob(File.join(include_path, "*")), File.join(File.dirname(__FILE__), "ext", "cmetrics"))
      FileUtils.touch(@checkpoint)
    end
  end

  def activate
    @recipe.activate
  end
end

spec = eval(File.read("cmetrics-ruby.gemspec"))

Rake::ExtensionTask.new('cmetrics', spec) do |ext|
  ext.ext_dir = 'ext/cmetrics'
  ext.cross_compile = true
  ext.lib_dir = File.join(*['lib', 'cmetrics', ENV['FAT_DIR']].compact)
  # cross_platform names are of MRI's platform name
  ext.cross_platform = ['x86-mingw32', 'x64-mingw32']
end

namespace :build do
  desc "Build CMetrics library"
  task :cmetrics do
    cmetrics = BuildCMetrics.new
    cmetrics.build
    cmetrics.activate
  end
end

task :clean do
  FileUtils.rm_f File.join(File.dirname(__FILE__), "ext", "cmetrics", "libcmetrics.a")
  FileUtils.rm_f File.join(File.dirname(__FILE__), "ext", "cmetrics", "cmetrics")
  FileUtils.rm_f File.join(File.dirname(__FILE__), "ext", "cmetrics", "monkey")
  FileUtils.rm_f File.join(File.dirname(__FILE__), Dir.glob(".*.installed"))
  FileUtils.rm_rf File.join(File.dirname(__FILE__), "ports")
  FileUtils.rm_rf File.join(File.dirname(__FILE__), "tmp")
end
