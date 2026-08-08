// Translated from solution.cpp.

var n: dynamic;

var v = cpp_array(100);

var s = cpp_array(100);

var c: dynamic;

func def()
{
  var s1: dynamic;
  var s2: dynamic;
  var s3: dynamic;
  read(s1, s2);
  var p = (s1.rfind("&") + 1);
  var q = s1.find("*");
  var v1: dynamic;
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
      var i = (n + 1);
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
      var i = (n + 1);
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
  var b = true;
  {
    var j = (n + 1);
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
    var i = (n + 1);
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

func of()
{
  var s1: dynamic;
  var s2: dynamic;
  read(s1);
  var p = (s1.rfind("&") + 1);
  var q = s1.find("*");
  var v1: dynamic;
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
  var b = true;
  {
    var i = (n + 1);
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

func main()
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
