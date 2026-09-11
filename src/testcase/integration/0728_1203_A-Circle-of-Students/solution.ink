// Translated from solution.cpp.

var cnt: dynamic = 0;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var h1: dynamic = cpp_array(100005);

var h2: dynamic = cpp_array(100007);

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(q);
  while (cpp_update(q, "--"))
  {
    read(n);
    var a: dynamic = cpp_array(n);
    var t: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(a[i]);
        if ((a[i] == 1))
        {
          t = i;
        }
        i += 1;
      }
    }
    var u: dynamic = 1;
    var nm: dynamic = 0;
    {
      i = t;
      while ((i < n))
      {
        if ((a[i] == u))
        {
        } else
        {
          nm += 1;
        }
        i += 1;
        u += 1;
      }
    }
    {
      i = 0;
      while ((i < t))
      {
        if ((a[i] == u))
        {
        } else
        {
          nm += 1;
        }
        i += 1;
        u += 1;
      }
    }
    var ut: dynamic = 0;
    u = 1;
    {
      i = t;
      while ((i >= 0))
      {
        if ((a[i] == u))
        {
        } else
        {
          ut += 1;
        }
        i -= 1;
        u += 1;
      }
    }
    {
      i = (n - 1);
      while ((i > t))
      {
        if ((a[i] == u))
        {
        } else
        {
          ut += 1;
        }
        i -= 1;
        u += 1;
      }
    }
    if (((nm > 0) && (ut > 0)))
    {
      write("NO\n");
    } else
    {
      write("YES\n");
    }
  }
}
