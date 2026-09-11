// Translated from solution.cpp.

var MAX: dynamic = cpp_expression("#inclu");

var INF: dynamic = cpp_expression("#includ");

var segN: dynamic = 1000000;

class data
{
  var value: dynamic = cpp_uninitialized();
  var ch: dynamic = cpp_uninitialized();
  func init() -> dynamic
  {
      value = INF;
      ch[0] = cpp_new();
      ch[1] = cpp_new();
      ch[0]->ch[0] = null;
      ch[1]->ch[0] = null;
    }
  func set(i: dynamic, x: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      var m: dynamic = (((l + r)) / 2);
      if ((ch[0] == null))
      {
        init();
      }
      if (((i < l) || (r <= i)))
      {
        return;
      }
      if ((x < value))
      {
        value = x;
      }
      if (((r - l) == 1))
      {
        return;
      }
      ch[0]->set(i, x, l, m);
      ch[1]->set(i, x, m, r);
    }
  func set(i: dynamic, x: dynamic) -> dynamic
  {
      set(i, x, 0, segN);
    }
  func min(a: dynamic, b: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      var m: dynamic = (((l + r)) / 2);
      if ((ch[0] == null))
      {
        init();
      }
      if (((b <= l) || (r <= a)))
      {
        return INF;
      }
      if (((a <= l) && (r <= b)))
      {
        return value;
      }
      var lc: dynamic = ch[0]->min(a, b, l, m);
      var rc: dynamic = ch[1]->min(a, b, m, r);
      return ( ((lc < rc)) ? lc : rc);
    }
  func min(a: dynamic, b: dynamic) -> dynamic
  {
      if (((0 <= a) && (a < b)))
      {
        return min(a, b, 0, segN);
      } else
      {
        return INF;
      }
    }
}

var rak: dynamic = cpp_array((MAX + 1));

var tmp: dynamic = cpp_array((MAX + 1));

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

func compare_sa(i: dynamic, j: dynamic) -> dynamic
{
  if ((rak[i] != rak[j]))
  {
    return ((rak[i] < rak[j]));
  } else
  {
    var ri: dynamic = ( (((i + k) <= n)) ? rak[(i + k)] : -1);
    var rj: dynamic = ( (((j + k) <= n)) ? rak[(j + k)] : -1);
    return (ri < rj);
  }
}

func construct_sa(s: dynamic, sa: dynamic) -> dynamic
{
  n = s.size();
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      sa[i] = i;
      rak[i] = ( ((i < n)) ? s[i] : -1);
      i += 1;
    }
  }
  {
    k = 1;
    while ((k <= n))
    {
      sort(sa, ((sa + n) + 1), compare_sa);
      tmp[sa[0]] = 0;
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          tmp[sa[i]] = (tmp[sa[(i - 1)]] + ( (compare_sa(sa[(i - 1)], sa[i])) ? 1 : 0));
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i <= n))
        {
          rak[i] = tmp[i];
          i += 1;
        }
      }
      k *= 2;
    }
  }
}

func search(target: dynamic, sa: dynamic, s: dynamic) -> dynamic
{
  var l: dynamic = 0;
  var r: dynamic = n;
  var m: dynamic = cpp_uninitialized();
  var size: dynamic = target.size();
  var res: dynamic = cpp_uninitialized();
  while ((l < r))
  {
    m = (((l + r)) / 2);
    res = s.compare(sa[m], size, target);
    if ((res >= 0))
    {
      r = m;
    } else
    {
      l = (m + 1);
    }
  }
  if ((s.compare(sa[l], size, target) < 0))
  {
    return (n + 1);
  }
  return l;
}

func search2(target: dynamic, sa: dynamic, s: dynamic) -> dynamic
{
  var l: dynamic = 0;
  var r: dynamic = n;
  var m: dynamic = cpp_uninitialized();
  var size: dynamic = target.size();
  var res: dynamic = cpp_uninitialized();
  while ((l < r))
  {
    m = (((l + r)) / 2);
    res = s.compare(sa[m], size, target);
    if ((res > 0))
    {
      r = m;
    } else
    {
      l = (m + 1);
    }
  }
  if ((s.compare(sa[l], size, target) <= 0))
  {
    return (n + 1);
  }
  return l;
}

var T: dynamic = cpp_uninitialized();

var T2: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var s2: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var sa: dynamic = cpp_array((MAX + 1));

var sa2: dynamic = cpp_array((MAX + 1));

func main() -> dynamic
{
  T.init();
  T2.init();
  read(s);
  s2 = s;
  reverse(s2.begin(), s2.end());
  construct_sa(s2, sa2);
  construct_sa(s, sa);
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      T.set(i, sa[i]);
      T2.set(i, sa2[i]);
      i += 1;
    }
  }
  read(m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(x, y);
      reverse(y.begin(), y.end());
      var xl: dynamic = search(x, sa, s);
      var xr: dynamic = search2(x, sa, s);
      var yl: dynamic = search(y, sa2, s2);
      var yr: dynamic = search2(y, sa2, s2);
      var xs: dynamic = x.size();
      var X: dynamic = T.min(xl, xr);
      var Y: dynamic = T2.min(yl, yr);
      if ((Y != INF))
      {
        Y = (n - Y);
      }
      if ((Y < (X + xs)))
      {
        write(0, "\n");
      } else if (((X == INF) || (Y == INF)))
      {
        write(0, "\n");
      } else
      {
        write((Y - X), cpp_char("\n"));
      }
      i += 1;
    }
  }
  return 0;
}
