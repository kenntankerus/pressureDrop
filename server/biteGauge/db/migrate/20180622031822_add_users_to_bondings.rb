class AddUsersToBondings < ActiveRecord::Migration[5.2]
  def change
    add_reference :bondings, :users, foreign_key: true
  end
end
