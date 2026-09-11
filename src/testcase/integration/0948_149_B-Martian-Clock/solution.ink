// Translated from solution.cpp.

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

func ok(p: dynamic) -> dynamic
{
  var f: dynamic = true;
  {
    var i: dynamic = 0;
    while ((i < a.size()))
    {
      f &= (a[i] < p);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < b.size()))
    {
      f &= (b[i] < p);
      i += 1;
    }
  }
  var h: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  h = cpp_assign(m, "=", 0);
  {
    var i: dynamic = 0;
    while ((i < a.size()))
    {
      h *= p;
      h += a[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < b.size()))
    {
      m *= p;
      m += b[i];
      i += 1;
    }
  }
  return ((f && ((m < 60))) && ((h < 24)));
}

func main() -> dynamic
{
  read(s);
  var i: dynamic = 0;
  while ((s[i] != cpp_char(":")))
  {
    if (((s[i] >= cpp_char("0")) && (s[i] <= cpp_char("9"))))
    {
      a.push_back((s[i] - cpp_char("0")));
    } else
    {
      a.push_back(((s[i] - cpp_char("A")) + 10));
    }
    i += 1;
  }
  s.erase(s.begin(), ((s.begin() + s.find(cpp_char(":"))) + 1));
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      if (((s[i] >= cpp_char("0")) && (s[i] <= cpp_char("9"))))
      {
        b.push_back((s[i] - cpp_char("0")));
      } else
      {
        b.push_back(((s[i] - cpp_char("A")) + 10));
      }
      i += 1;
    }
  }
  if (ok(60))
  {
    write(-1, "\n");
    return 0;
  }
  {
    var i: dynamic = 2;
    while ((i < 60))
    {
      if (ok(i))
      {
        v.push_back(i);
      }
      i += 1;
    }
  }
  if ((v.size() == 0))
  {
    write(0, "\n");
    return 0;
  }
  write(v[0]);
  {
    var i: dynamic = 1;
    while ((i < v.size()))
    {
      write(cpp_char(" "), v[i]);
      i += 1;
    }
  }
  write("\n");
  return 0;
}
