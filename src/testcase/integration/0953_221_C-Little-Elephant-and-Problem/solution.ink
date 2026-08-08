// Translated from solution.cpp.

func swap(x: dynamic, y: dynamic)
{
  var t = x;
  x = y;
  y = t;
}

func main()
{
  var n: dynamic;
  read(n);
  var p = cpp_new();
  var k: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(p[i]);
      k.push_back(p[i]);
      i += 1;
    }
  }
  sort(k.begin(), k.end());
  var count = 0;
  {
    var i = 0;
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
