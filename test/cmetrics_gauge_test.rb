# frozen_string_literal: true

require "test_helper"

class CMetricsGaugeTest < Test::Unit::TestCase
  sub_test_case "gauge" do
    setup do
      @gauge = CMetrics::Gauge.new
      @gauge.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])
    end

    def test_gauge
      assert_equal nil, @gauge.val
      assert_true @gauge.set 2.0

      assert_equal 2.0, @gauge.val

      assert_true @gauge.inc
      assert_equal 3.0, @gauge.val

      assert_true @gauge.sub 2.0
      assert_equal 1.0, @gauge.val

      assert_true @gauge.dec
      assert_equal 0.0, @gauge.val
      puts @gauge.to_prometheus
    end

    def test_labels
      assert_true @gauge.inc(["localhost", "cmetrics"])
      assert_equal 1.0, @gauge.val(["localhost", "cmetrics"])

      assert_true @gauge.add(10, ["localhost", "test"])
      assert_equal 10, @gauge.val(["localhost", "test"])

      assert_true @gauge.sub(2.5, ["localhost", "test"])
      assert_equal 7.5, @gauge.val(["localhost", "test"])

      puts @gauge.to_prometheus
      assert_not_nil @gauge.to_msgpack
    end
  end

  sub_test_case "gauge w/ one symbol" do
    setup do
      @gauge = CMetrics::Gauge.new
      @gauge.create("kubernetes", "network", "load", "Network load", :hostname)
    end

    def test_gauge
      assert_equal nil, @gauge.val
      assert_true @gauge.inc
      assert_equal 1.0, @gauge.val

      assert_true @gauge.add 2.0
      assert_equal 3.0, @gauge.val
      puts @gauge.to_prometheus
    end

    def test_label
      assert_true @gauge.inc(:localhost)
      assert_equal 1.0, @gauge.val(:localhost)

      assert_true @gauge.add(10.55, :k8s_worker)
      assert_equal 10.55, @gauge.val(:k8s_worker)

      assert_true @gauge.set(12.15, :k8s_worker)
      assert_true @gauge.set(1, :k8s_worker)
      puts @gauge.to_prometheus
    end
  end

  sub_test_case "gauge w/ symbols" do
    setup do
      @gauge = CMetrics::Gauge.new
      @gauge.create("kubernetes", "network", "load", "Network load", [:hostname, :app])
    end

    def test_gauge
      assert_equal nil, @gauge.val
      assert_true @gauge.set 2.0

      assert_equal 2.0, @gauge.val

      assert_true @gauge.inc
      assert_equal 3.0, @gauge.val

      assert_true @gauge.sub 2.0
      assert_equal 1.0, @gauge.val

      assert_true @gauge.dec
      assert_equal 0.0, @gauge.val
      puts @gauge.to_prometheus
    end

    def test_labels
      assert_true @gauge.inc([:localhost, :cmetrics])
      assert_equal 1.0, @gauge.val([:localhost, :cmetrics])

      assert_true @gauge.add(10, [:localhost, :test])
      assert_equal 10, @gauge.val([:localhost, :test])

      assert_true @gauge.sub(2.5, [:localhost, :test])
      assert_equal 7.5, @gauge.val([:localhost, :test])

      puts @gauge.to_prometheus
    end
  end
end
