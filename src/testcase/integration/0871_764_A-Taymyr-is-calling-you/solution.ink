// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  var c: dynamic = 0;
  var v: dynamic = cpp_uninitialized();
  read(n, m, z);
  {
    var i: dynamic = 1;
    while ((((i * n)) <= z))
    {
      v.push_back((i * n));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
