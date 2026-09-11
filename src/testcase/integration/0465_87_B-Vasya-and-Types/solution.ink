// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(100);

var s: dynamic = cpp_array(100);

var c: dynamic = cpp_uninitialized();

func def() -> dynamic
{
  var s1: dynamic = cpp_uninitialized();
  var s2: dynamic = cpp_uninitialized();
  var s3: dynamic = cpp_uninitialized();
  read(s1, s2);
  var p: dynamic = (s1.rfind("&") + 1);
  var q: dynamic = s1.find("*");
  var v1: dynamic = cpp_uninitialized();
  if ((q < 0))
  {
    v1 = (0 - p);
    q = 1000;
  } else
  {
    v1 = (((s1.size() - q)) - p);
  }
  s3 = s1.substr(p, (q - p));
  if ((s3 == "void"))
  {
    {
      var i: dynamic = (n + 1);
      while ((i < 100))
      {
        if ((s[i] == s2))
        {
          v[i] = v1;
          return 0;
        }
        i += 1;
      }
    }
    s[n] = s2;
    v[n] = v1;
    return 0;
  } else if ((s3 == "errtype"))
  {
    {
      var i: dynamic = (n + 1);
      while ((i < 100))
      {
        if ((s[i] == s2))
        {
          v[i] = -1;
          return 0;
        }
        i += 1;
      }
    }
    s[n] = s2;
    v[n] = -1;
    return 0;
  }
  var b: dynamic = true;
  {
    var j: dynamic = (n + 1);
    while ((j < 100))
    {
      if ((s[j] == s3))
      {
        if ((v[j] < 0))
        {
          v1 = -1;
        } else
        {
          v1 += v[j];
        }
        b = false;
      }
      j += 1;
    }
  }
  if (b)
  {
    v1 = -1;
  }
  {
    var i: dynamic = (n + 1);
    while ((i < 100))
    {
      if ((s[i] == s2))
      {
        v[i] = v1;
        return 0;
      }
      i += 1;
    }
  }
  s[n] = s2;
  v[n] = v1;
  return 0;
}

func of() -> dynamic
{
  var s1: dynamic = cpp_uninitialized();
  var s2: dynamic = cpp_uninitialized();
  read(s1);
  var p: dynamic = (s1.rfind("&") + 1);
  var q: dynamic = s1.find("*");
  var v1: dynamic = cpp_uninitialized();
  if ((q < 0))
  {
    v1 = (0 - p);
    q = 1000;
  } else
  {
    v1 = (((s1.size() - q)) - p);
  }
  s2 = s1.substr(p, (q - p));
  if ((s2 == "void"))
  {
    write("void");
    while (cpp_update(v1, "--"))
    {
      write("*");
    }
    write("\n");
    return 0;
  }
  if ((s2 == "errtype"))
  {
    write("errtype", "\n");
    return 0;
  }
  var b: dynamic = true;
  {
    var i: dynamic = (n + 1);
    while ((i < 100))
    {
      if ((s[i] == s2))
      {
        if ((v[i] < 0))
        {
          v1 = -1;
        } else
        {
          v1 += v[i];
        }
        if ((v1 < 0))
        {
          write("errtype", "\n");
        } else
        {
          write("void");
          while (cpp_update(v1, "--"))
          {
            write("*");
          }
          write("\n");
        }
        b = false;
      }
      i += 1;
    }
  }
  if (b)
  {
    write("errtype", "\n");
  }
  return 0;
}

func main() -> dynamic
{
  read(n);
  while (cpp_update(n, "--"))
  {
    read(c);
    if ((c == "typedef"))
    {
      def();
    } else
    {
      of();
    }
  }
  return 0;
}
