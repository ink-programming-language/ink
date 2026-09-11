// Translated from solution.cpp.

var p: dynamic = [];

var arr: dynamic = [];

func prime() -> dynamic
{
  p[0] = cpp_assign(p[1], "=", 1);
  {
    var i: dynamic = 2;
    while ((i < 10000))
    {
      if ((p[i] == 0))
      {
        {
          var j: dynamic = (i * i);
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
  var m: dynamic = 0;
  {
    var i: dynamic = 2;
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
    var i: dynamic = 0;
    while ((i < 100))
    {
      write(i, ":", arr[i], "\n");
      i += 1;
    }
  }
}

func recurse(a: dynamic, b: dynamic, v: dynamic) -> dynamic
{
  if ((a >= b))
  {
    if ((a == b))
    {
      write("YES", "\n");
      var s: dynamic = v.size();
      write(s, "\n");
      var it: dynamic = cpp_uninitialized();
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
  var t: dynamic = cpp_uninitialized();
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

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var v: dynamic = cpp_uninitialized();
  v.push_back(n);
  var f: dynamic = recurse(n, m, v);
  if ((!f))
  {
    write("NO");
  }
}
