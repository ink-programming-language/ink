// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  var m: dynamic;
  var z: dynamic;
  var c = 0;
  var v: dynamic;
  read(n, m, z);
  {
    var i = 1;
    while ((((i * n)) <= z))
    {
      v.push_back((i * n));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((((i * m)) <= z))
    {
      if (binary_search(v.begin(), v.end(), (i * m)))
      {
        c += 1;
      }
      i += 1;
    }
  }
  write(c, "\n");
  return 0;
}
