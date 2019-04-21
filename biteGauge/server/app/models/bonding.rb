class Bonding < ApplicationRecord
  belongs_to :user
  validates :user, :presence => true
  attr_accessor :time, :data
end
