require 'helper'

class BaseTests < Test::Unit::TestCase
  def setup
    @start    = [250.0] * 8
    @mult     = [5, 10, 20, 25, 50, 100, 125, 250]
  end

  def self.build_tests(klass)
    define_method :test_multiplication do
      expected = @start.zip(@mult).each_with_object(:*).map(&:reduce)
      result   = @start_v * @mult_v

      assert_equal(expected, result.to_a)
    end

    define_method :test_division do
      expected = @start.zip(@mult).each_with_object(:/).map(&:reduce)
      result   = @start_v / @mult_v

      assert_equal(expected, result.to_a)
    end

    define_method :test_addition do
      expected = @start.zip(@mult).each_with_object(:+).map(&:reduce)
      result   = @start_v + @mult_v

      assert_equal(expected, result.to_a)
    end

    define_method :test_subtraction do
      expected = @start.zip(@mult).each_with_object(:-).map(&:reduce)
      result   = @start_v - @mult_v

      assert_equal(expected, result.to_a)
    end

    define_method :test_unaligned_vectors_wont_div_by_zero do
      unaligned1 = [3,4,5,6,7]
      unaligned2 = [1,2,3,4,5]
      unaligned3 = [2.0] * 5
      expected   = [1.0] * 5

      vector1 = klass.new(unaligned1)
      vector2 = klass.new(unaligned2)
      vector3 = klass.new(unaligned3)

      intermediary = vector1 - vector2
      assert_equal(unaligned3, intermediary.to_a)

      result = intermediary / vector3
      assert_equal(expected, result.to_a)
    end

    define_method :test_cannot_operate_between_even_and_odd_length_vectors do
      veven = klass.new([1,2,3,4])
      vodd  = klass.new([1,2,3,4,5])
      assert_raises(ArgumentError) { veven * vodd }
    end

    define_method :test_creation_and_extraction do
      expected = (1..8).to_a
      vector = klass.new(expected)

      assert_equal(8, vector.length)
      assert_equal(expected, vector.to_a)
    end
  end

=begin

  def test_cannot_create_arrays_shorter_than_two
    assert_raises(ArgumentError) { SIMD::FloatArray.new([]) }
    assert_raises(ArgumentError) { SIMD::FloatArray.new([1]) }
  end

=end
end
