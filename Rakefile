# frozen_string_literal: true

require "bundler/gem_tasks"
require "rake/extensiontask"

task build: :compile

GEMSPEC = Gem::Specification.load("shareable_mutex.gemspec")

Rake::ExtensionTask.new("shareable_mutex", GEMSPEC) do |ext|
  ext.lib_dir = "lib/shareable_mutex"
end

task default: %i[clobber compile]
