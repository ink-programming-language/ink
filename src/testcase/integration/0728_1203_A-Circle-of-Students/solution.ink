// Translated from solution.cpp.

var cnt = 0;

var n: dynamic;

var m: dynamic;

var h1 = cpp_array(100005);

var h2 = cpp_array(100007);

func main()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var l: dynamic;
  var n: dynamic;
  var m: dynamic;
  var q: dynamic;
  read(q);
  while (cpp_update(q, "--"))
  {
    read(n);
    var a = cpp_array(n);
    var t = 0;
    {
      var i = 0;
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
    var u = 1;
    var nm = 0;
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
    var ut = 0;
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
