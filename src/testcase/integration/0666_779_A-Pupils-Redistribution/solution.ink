// Translated from solution.cpp.

var n: dynamic;

var a = cpp_array(200);

var b = cpp_array(200);

var t: dynamic;

var ans: dynamic;

var mp: dynamic;

var c: dynamic;

var d: dynamic;

var s: dynamic;

func main()
{
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      mp[a[i]] += 1;
      c[a[i]] += 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(b[i]);
      mp[b[i]] += 1;
      d[b[i]] += 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < mp.size()))
    {
      if (((mp[i] % 2) == 1))
      {
        write(-1);
        t += 1;
        break;
      }
      i += 1;
    }
  }
  if ((t == 0))
  {
    {
      var i = 0;
      while ((i < mp.size()))
      {
        ans += abs((c[i] - d[i]));
        i += 1;
      }
    }
    write((ans / 4));
  }
}
