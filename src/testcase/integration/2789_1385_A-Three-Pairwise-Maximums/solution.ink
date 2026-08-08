// Translated from solution.cpp.

var mod = (1e9 + 7);

func solve()
{
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  read(x, y, z);
  if (((x == y) && (y == z)))
  {
    write("YES", "\n", x, " ", x, " ", x);
  } else if (((x == y) && (x > z)))
  {
    write("YES", "\n", x, " ", z, " ", z);
  } else if (((x == z) && (x > y)))
  {
    write("YES", "\n", x, " ", y, " ", y);
  } else if (((y == z) && (y > x)))
  {
    write("YES", "\n", x, " ", x, " ", y);
  } else
  {
    write("NO");
  }
  write("\n");
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
