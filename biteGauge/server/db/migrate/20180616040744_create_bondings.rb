class CreateBondings < ActiveRecord::Migration[5.2]
  def change
    create_table :bondings do |t|
      t.datetime :time
      t.text :data

      t.timestamps
    end
  end
end
