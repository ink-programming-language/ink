// Translated from solution.cpp.

func input(v: dynamic, n: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
}

func main()
{
  var t: dynamic;
  var n: dynamic;
  read(t);
  var Case = 0;
  while (cpp_update(t, "--"))
  {
    read(n);
    var mp1: dynamic;
    {
      var i = 0;
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
        var z = (((v2[i] + i)) % n);
        mp1[z] += 1;
        i += 1;
      }
    }
    var flag = 1;
    {
      var i = 0;
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
