// Translated from solution.cpp.

func swap(x: dynamic, y: dynamic) -> dynamic
{
  var t: dynamic = x;
  x = y;
  y = t;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var p: dynamic = cpp_new();
  var k: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(p[i]);
      k.push_back(p[i]);
      i += 1;
    }
  }
  sort(k.begin(), k.end());
  var count: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((p[i] != k[i]))
      {
        count += 1;
      }
      i += 1;
    }
  }
  if ((count < 3))
  {
    write("YES");
  } else
  {
    write("NO");
  }
  cpp_delete(p);
  return 0;
}
