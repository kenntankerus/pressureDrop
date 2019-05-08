class User < ApplicationRecord
  has_many :bondings
  attr_accessor :first_name, :last_name, :dob
end
