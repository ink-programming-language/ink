// Translated from solution.cpp.

var f = cpp_construct(100001);

func comp(i: dynamic)
{
  var b = 0;
  var d = 0;
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

func main()
{
  var m: dynamic;
  var i: dynamic;
  var n: dynamic;
  var h = cpp_array(65);
  var j: dynamic;
  var s: dynamic;
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
