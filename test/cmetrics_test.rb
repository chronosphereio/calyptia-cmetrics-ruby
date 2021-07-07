# frozen_string_literal: true

require "test_helper"

class CMetricsTest < Test::Unit::TestCase
  test "VERSION" do
    assert do
      ::CMetrics.const_defined?(:VERSION)
    end
  end

  test "library version" do
    assert do
      ::CMetrics.library_version
    end
  end
end
