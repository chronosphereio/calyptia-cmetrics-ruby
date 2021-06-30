# frozen_string_literal: true

require "test_helper"

class CMetricsCounterTest < Test::Unit::TestCase
  sub_test_case "counter" do
    setup do
      @counter = CMetrics::Counter.new
      @counter.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])
    end

    def test_counter
      assert_equal nil, @counter.val
      assert_true @counter.inc
      assert_equal 1.0, @counter.val

      assert_true @counter.add 2.0
      assert_equal 3.0, @counter.val
      puts @counter.to_prometheus
      assert_not_nil @counter.to_s
    end

    def test_labels
      assert_true @counter.inc(["localhost", "cmetrics"])
      assert_equal 1.0, @counter.val(["localhost", "cmetrics"])

      assert_true @counter.add(10.55, ["localhost", "test"])
      assert_equal 10.55, @counter.val(["localhost", "test"])

      assert_true @counter.set(12.15, ["localhost", "test"])
      assert_false @counter.set(1, ["localhost", "test"])
      puts @counter.to_prometheus
      assert_not_nil @counter.to_s
    end

    def test_prometheus
      counter = CMetrics::Counter.new
      counter.create("cmt", "labels", "test", "Static labels test", ["host", "app"])

      counter.inc
      counter.inc(["calyptia.com", "cmetrics"])
      counter.inc(["calyptia.com", "cmetrics"])

      expected = <<-EOC
# HELP cmt_labels_test Static labels test
# TYPE cmt_labels_test counter
cmt_labels_test 1 \\d+
cmt_labels_test{host="calyptia.com",app="cmetrics"} 2 \\d+
EOC
      assert_match(/#{expected}/, counter.to_prometheus)

      assert_true counter.add_label("dev", "Calyptia")
      assert_true counter.add_label("lang", "C")

      expected2 = <<-EOC
# HELP cmt_labels_test Static labels test
# TYPE cmt_labels_test counter
cmt_labels_test{dev="Calyptia",lang="C"} 1 \\d+
cmt_labels_test{dev="Calyptia",lang="C",host="calyptia.com",app="cmetrics"} 2 \\d+
EOC
      assert_match(/#{expected2}/, counter.to_prometheus)
    end
  end

  sub_test_case "counter w/ one symbol" do
    setup do
      @counter = CMetrics::Counter.new
      @counter.create("kubernetes", "network", "load", "Network load", :hostname)
    end

    def test_counter
      assert_equal nil, @counter.val
      assert_true @counter.inc
      assert_equal 1.0, @counter.val

      assert_true @counter.add 2.0
      assert_equal 3.0, @counter.val
      puts @counter.to_prometheus
      assert_not_nil @counter.to_s
    end

    def test_label
      assert_true @counter.inc(:localhost)
      assert_equal 1.0, @counter.val(:localhost)

      assert_true @counter.add(10.55, :k8s_worker)
      assert_equal 10.55, @counter.val(:k8s_worker)

      assert_true @counter.set(12.15, :k8s_worker)
      assert_false @counter.set(1, :k8s_worker)
      puts @counter.to_prometheus
      assert_not_nil @counter.to_s
    end
  end

  sub_test_case "counter w/ symbols" do
    setup do
      @counter = CMetrics::Counter.new
      @counter.create("kubernetes", "network", "load", "Network load", [:hostname, :app])
    end

    def test_counter
      assert_equal nil, @counter.val
      assert_true @counter.inc
      assert_equal 1.0, @counter.val

      assert_true @counter.add 2.0
      assert_equal 3.0, @counter.val
      puts @counter.to_prometheus
      assert_not_nil @counter.to_s
    end

    def test_labels
      assert_true @counter.inc([:localhost, :cmetrics])
      assert_equal 1.0, @counter.val([:localhost, :cmetrics])

      assert_true @counter.add(10.55, [:localhost, :test])
      assert_equal 10.55, @counter.val([:localhost, :test])

      assert_true @counter.set(12.15, [:localhost, :test])
      assert_false @counter.set(1, [:localhost, :test])
      puts @counter.to_prometheus
      assert_not_nil @counter.to_msgpack
      assert_not_nil @counter.to_s
    end
  end

  sub_test_case "not enough initialized" do
    setup do
      @counter = CMetrics::Counter.new
    end

    def test_error
      assert_raise(RuntimeError) do
        @counter.inc
      end

      assert_raise(RuntimeError) do
        @counter.add 2.0
      end

      assert_raise(RuntimeError) do
        @counter.set 2.0
      end

      assert_raise(RuntimeError) do
        @counter.val
      end
    end
  end
end
