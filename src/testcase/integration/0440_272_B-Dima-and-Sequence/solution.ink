// Translated from solution.cpp.

var f: dynamic = cpp_construct(100001);

func comp(i: dynamic) -> dynamic
{
  var b: dynamic = 0;
  var d: dynamic = 0;
  while ((i > 0))
  {
    if ((i & (1 == 1)))
    {
      b += 1;
    }
    i = (i >> 1);
  }
  return b;
}

func main() -> dynamic
{
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_array(65);
  var j: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < 65))
    {
      h[i] = 0;
      h[(i + 1)] = 0;
      i += 2;
    }
  }
  read(n);
  {
    i = 0;
    while ((i < n))
    {
      read(f[i]);
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      h[comp(f[i])] += 1;
      i += 1;
    }
  }
  s = 0;
  {
    i = 0;
    while ((i < 65))
    {
      j = h[i];
      j = (j * ((j - 1)));
      j = (j / 2);
      s += j;
      i += 1;
    }
  }
  write(s);
  return 0;
}
