// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(200);

var b: dynamic = cpp_array(200);

var t: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var mp: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      mp[a[i]] += 1;
      c[a[i]] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(b[i]);
      mp[b[i]] += 1;
      d[b[i]] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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
      var i: dynamic = 0;
      while ((i < mp.size()))
      {
        ans += abs((c[i] - d[i]));
        i += 1;
      }
    }
    write((ans / 4));
  }
}
