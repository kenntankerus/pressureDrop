# drop the users table to use devise for sutherization
class DropUsersTable < ActiveRecord::Migration[6.0]
  def change
    drop_table :users
  end
end
