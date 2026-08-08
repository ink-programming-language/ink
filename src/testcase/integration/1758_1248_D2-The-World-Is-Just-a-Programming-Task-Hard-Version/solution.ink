// Translated from solution.cpp.

class msp
{
  var x: dynamic;
  var y: dynamic;
  var t: dynamic;
}

var ans: dynamic;

var n: dynamic;

func cntans(s: dynamic)
{
  var t = 0;
  var f = 0;
  var minm = 0x3f3f3f3f;
  {
    var i = 0;
    while ((i < s.length()))
    {
      if ((s[i] == cpp_char("(")))
      {
        t += 1;
      } else
      {
        t -= 1;
      }
      if ((t < minm))
      {
        minm = t;
        f = i;
      }
      i += 1;
    }
  }
  f += 1;
  s = (s.substr(f, (s.length() - f)) + s.substr(0, f));
  return f;
}

var d = cpp_array(301000);

var f = cpp_array(301000);

func work1()
{
  var l = 0;
  var r = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((d[i] == 1))
      {
        l = i;
        break;
      }
      i += 1;
    }
  }
  {
    var i = n;
    while ((i >= 1))
    {
      if ((d[i] == -1))
      {
        r = i;
        break;
      }
      i -= 1;
    }
  }
  if ((l < r))
  {
    var t = 0;
    {
      var i = l;
      while ((i <= r))
      {
        if ((f[i] == 0))
        {
          t += 1;
        }
        i += 1;
      }
    }
    if ((t > ans.t))
    {
      ans.x = l;
      ans.y = r;
      ans.t = t;
    }
  }
}

func work2()
{
  var l = 0x3f3f3f3f;
  var r = 0;
  var t = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((f[i] < 1))
      {
        if ((t > ans.t))
        {
          ans.x = l;
          ans.y = i;
          ans.t = t;
        }
        t = 0;
        l = 0x3f3f3f3f;
      } else
      {
        if ((l == 0x3f3f3f3f))
        {
          l = i;
        }
        if ((f[i] == 1))
        {
          t += 1;
        }
      }
      i += 1;
    }
  }
}

var orit: dynamic;

func work3()
{
  var l = 0x3f3f3f3f;
  var r = 0;
  var t = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((f[i] < 2))
      {
        if (((t + orit) > ans.t))
        {
          ans.x = l;
          ans.y = i;
          ans.t = (t + orit);
        }
        t = 0;
        l = 0x3f3f3f3f;
      } else
      {
        if ((l == 0x3f3f3f3f))
        {
          l = i;
        }
        if ((f[i] == 2))
        {
          t += 1;
        }
      }
      i += 1;
    }
  }
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  read(n);
  var s: dynamic;
  read(s);
  ans.x = cpp_assign(ans.y, "=", 0);
  var dta = cntans(s);
  {
    var i = 1;
    while ((i <= n))
    {
      d[i] = (if ((s[(i - 1)] == cpp_char("("))) 1 else -1);
      f[i] = (f[(i - 1)] + d[i]);
      if ((f[i] == 0))
      {
        ans.t += 1;
      }
      i += 1;
    }
  }
  orit = ans.t;
  if ((f[n] != 0))
  {
    write(0, cpp_char("\n"), 1, cpp_char(" "), 1, cpp_char("\n"));
    return 0;
  }
  work1();
  work2();
  work3();
  ans.x += dta;
  ans.y += dta;
  ans.x = ((((ans.x - 1)) % n) + 1);
  ans.y = ((((ans.y - 1)) % n) + 1);
  write(ans.t, cpp_char("\n"), ans.x, cpp_char(" "), ans.y);
  return 0;
}
