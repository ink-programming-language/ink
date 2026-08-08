// Translated from solution.cpp.

var p = [];

var arr = [];

func prime()
{
  p[0] = cpp_assign(p[1], "=", 1);
  {
    var i = 2;
    while ((i < 10000))
    {
      if ((p[i] == 0))
      {
        {
          var j = (i * i);
          while ((j < 10000))
          {
            if ((p[j] == 0))
            {
              p[j] = 1;
            }
            j += i;
          }
        }
      }
      i += 1;
    }
  }
  var m = 0;
  {
    var i = 2;
    while ((i < 10000))
    {
      arr[i] = arr[(i - 1)];
      if ((p[i] == 0))
      {
        arr[i] += i;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < 100))
    {
      write(i, ":", arr[i], "\n");
      i += 1;
    }
  }
}

func recurse(a: dynamic, b: dynamic, v: dynamic)
{
  if ((a >= b))
  {
    if ((a == b))
    {
      write("YES", "\n");
      var s = v.size();
      write(s, "\n");
      var it: dynamic;
      {
        it = v.begin();
        while ((it != v.end()))
        {
          write((*it), " ");
          it += 1;
        }
      }
      return 1;
    }
    return 0;
  }
  var t: dynamic;
  t = v;
  t.push_back((a * 2));
  if (recurse((a * 2), b, t))
  {
    return 1;
  }
  t = v;
  t.push_back(((a * 10) + 1));
  if (recurse(((a * 10) + 1), b, t))
  {
    return 1;
  }
  return 0;
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var v: dynamic;
  v.push_back(n);
  var f = recurse(n, m, v);
  if ((!f))
  {
    write("NO");
  }
}
