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

def linux?
  RUBY_PLATFORM =~ /linux/
end

def windows?
  RUBY_PLATFORM =~ /mingw|mswin/
end

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
      url: "file://#{File.dirname(__FILE__)}/ext/#{@recipe.name}-#{@recipe.version}.tar.gz",
      sha256sum: "0ec23bb8848e2c1cdcab7140cc8784036c75795d5883e0c599f4a3e80911af5a",
    }
  end

  def build
    unless File.exist?(@checkpoint)
      @recipe.cook
      libcmetrics_path = File.join(File.dirname(__FILE__), "ports/#{@recipe.host}/cmetrics/#{@version}/lib/libcmetrics.a")
      FileUtils.cp(libcmetrics_path, File.join(File.dirname(__FILE__), "ext", "cmetrics", "libcmetrics.a"))
      libmpack_path = File.join(File.dirname(__FILE__), "ports/#{@recipe.host}/cmetrics/#{@version}/lib/libmpack.a")
      FileUtils.cp(libmpack_path, File.join(File.dirname(__FILE__), "ext", "cmetrics", "libmpack.a"))
      libxxhash_path = File.join(File.dirname(__FILE__), "ports/#{@recipe.host}/cmetrics/#{@version}/lib/libxxhash.a")
      FileUtils.cp(libxxhash_path, File.join(File.dirname(__FILE__), "ext", "cmetrics", "libxxhash.a"))
      include_path = File.join(File.dirname(__FILE__), "ports/#{@recipe.host}/cmetrics/#{@version}/include/")
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
  FileUtils.rm_f File.join(File.dirname(__FILE__), "ext", "cmetrics", "lib*.a")
  FileUtils.rm_rf File.join(File.dirname(__FILE__), "ext", "cmetrics", "cmetrics")
  FileUtils.rm_rf File.join(File.dirname(__FILE__), "ext", "cmetrics", "monkey")
  FileUtils.rm_f File.join(File.dirname(__FILE__), Dir.glob(".*.installed"))
  FileUtils.rm_rf File.join(File.dirname(__FILE__), "ports")
  FileUtils.rm_rf File.join(File.dirname(__FILE__), "tmp")
end
