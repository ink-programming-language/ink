// Translated from solution.cpp.

var MAX = cpp_expression("#inclu");

var INF = cpp_expression("#includ");

var segN = 1000000;

class data
{
  var value: dynamic;
  var ch: dynamic;
  func init()
  {
      value = INF;
      ch[0] = cpp_new();
      ch[1] = cpp_new();
      ch[0]->ch[0] = null;
      ch[1]->ch[0] = null;
    }
  func set(i: dynamic, x: dynamic, l: dynamic, r: dynamic)
  {
      var m = (((l + r)) / 2);
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
  func set(i: dynamic, x: dynamic)
  {
      set(i, x, 0, segN);
    }
  func min(a: dynamic, b: dynamic, l: dynamic, r: dynamic)
  {
      var m = (((l + r)) / 2);
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
      var lc = ch[0]->min(a, b, l, m);
      var rc = ch[1]->min(a, b, m, r);
      return (if ((lc < rc)) lc else rc);
    }
  func min(a: dynamic, b: dynamic)
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

var rak = cpp_array((MAX + 1));

var tmp = cpp_array((MAX + 1));

var n: dynamic;

var k: dynamic;

func compare_sa(i: dynamic, j: dynamic)
{
  if ((rak[i] != rak[j]))
  {
    return ((rak[i] < rak[j]));
  } else
  {
    var ri = (if (((i + k) <= n)) rak[(i + k)] else -1);
    var rj = (if (((j + k) <= n)) rak[(j + k)] else -1);
    return (ri < rj);
  }
}

func construct_sa(s: dynamic, sa: dynamic)
{
  n = s.size();
  {
    var i = 0;
    while ((i <= n))
    {
      sa[i] = i;
      rak[i] = (if ((i < n)) s[i] else -1);
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
        var i = 1;
        while ((i <= n))
        {
          tmp[sa[i]] = (tmp[sa[(i - 1)]] + (if (compare_sa(sa[(i - 1)], sa[i])) 1 else 0));
          i += 1;
        }
      }
      {
        var i = 0;
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

func search(target: dynamic, sa: dynamic, s: dynamic)
{
  var l = 0;
  var r = n;
  var m: dynamic;
  var size = target.size();
  var res: dynamic;
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

func search2(target: dynamic, sa: dynamic, s: dynamic)
{
  var l = 0;
  var r = n;
  var m: dynamic;
  var size = target.size();
  var res: dynamic;
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

var T: dynamic;

var T2: dynamic;

var s: dynamic;

var s2: dynamic;

var x: dynamic;

var y: dynamic;

var m: dynamic;

var sa = cpp_array((MAX + 1));

var sa2 = cpp_array((MAX + 1));

func main()
{
  T.init();
  T2.init();
  read(s);
  s2 = s;
  reverse(s2.begin(), s2.end());
  construct_sa(s2, sa2);
  construct_sa(s, sa);
  {
    var i = 0;
    while ((i <= n))
    {
      T.set(i, sa[i]);
      T2.set(i, sa2[i]);
      i += 1;
    }
  }
  read(m);
  {
    var i = 0;
    while ((i < m))
    {
      read(x, y);
      reverse(y.begin(), y.end());
      var xl = search(x, sa, s);
      var xr = search2(x, sa, s);
      var yl = search(y, sa2, s2);
      var yr = search2(y, sa2, s2);
      var xs = x.size();
      var X = T.min(xl, xr);
      var Y = T2.min(yl, yr);
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
