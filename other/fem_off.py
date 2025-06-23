from petsys import daqd, fe_power

conn = daqd.Connection()

# fe_power.get_fem_power_status(conn, 0, 0)
fe_power.set_fem_power(conn, 0, 0, "off")
# fe_power.get_fem_power_status(conn, 0, 0)

