// Translated from solution.cpp.

func input(v: dynamic, n: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  read(t);
  var Case: dynamic = 0;
  while (cpp_update(t, "--"))
  {
    read(n);
    var mp1: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        mp1.insert([i, 0]);
        read(v[i]);
        if ((v[i] >= 0))
        {
          v2[i] = (v[i] % n);
        } else
        {
          v2[i] = (n - ((abs(v[i])) % n));
        }
        var z: dynamic = (((v2[i] + i)) % n);
        mp1[z] += 1;
        i += 1;
      }
    }
    var flag: dynamic = 1;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if ((mp1[i] == 0))
        {
          flag = 0;
          break;
        }
        i += 1;
      }
    }
    if ((flag == 0))
    {
      write("NO\n");
    } else
    {
      write("YES\n");
    }
  }
  return 0;
}
