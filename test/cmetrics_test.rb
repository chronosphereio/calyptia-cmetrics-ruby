# frozen_string_literal: true

require "test_helper"

class CMetricsTest < Test::Unit::TestCase
  test "VERSION" do
    assert do
      ::CMetrics.const_defined?(:VERSION)
    end
  end
end
