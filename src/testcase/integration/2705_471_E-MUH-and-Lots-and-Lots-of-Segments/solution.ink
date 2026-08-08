// Translated from solution.cpp.

class event
{
  var x: dynamic;
  var type_cpp: dynamic;
  var y: dynamic;
  var righty: dynamic;
}

class P
{
  var right: dynamic;
  var dsu: dynamic;
  var number: dynamic;
}

var xbegin = cpp_array(1000000);

var compcount: dynamic;

var fen = cpp_array(1000000);

var xend = cpp_array(1000000);

var ybegin = cpp_array(1000000);

var yend = cpp_array(1000000);

var realx = cpp_array(1000000);

var realy = cpp_array(1000000);

var parent = cpp_array(1000000);

var events = cpp_array(1000000);

var ans = cpp_array(1000000);

var best: dynamic;

var xx = cpp_array(1000000);

var yy = cpp_array(1000000);

var lefts: dynamic;

var s: dynamic;

var v: dynamic;

var n: dynamic;

func event_vertical(x: dynamic, yl: dynamic, yr: dynamic)
{
  v += 1;
  events[v].x = x;
  events[v].type_cpp = 2;
  events[v].y = yl;
  events[v].righty = yr;
}

func event_horizantal_begin(x: dynamic, y: dynamic, xx: dynamic)
{
  v += 1;
  events[v].x = x;
  events[v].type_cpp = 1;
  events[v].y = y;
  events[v].righty = xx;
}

func event_horizantal_end(x: dynamic, y: dynamic)
{
  v += 1;
  events[v].x = x;
  events[v].type_cpp = 3;
  events[v].y = y;
  events[v].righty = 0;
}

func CMP(i: dynamic, j: dynamic)
{
  return (((i.x < j.x) || (((i.x == j.x) && (i.type_cpp < j.type_cpp)))));
}

func compressX()
{
  {
    var i = 1;
    while ((i <= n))
    {
      xx[i] = make_pair(xbegin[i], i);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      xx[(i + n)] = make_pair(xend[i], (i + n));
      i += 1;
    }
  }
  sort((xx + 1), ((xx + (2 * n)) + 1));
  var v = 1;
  xx[0] = make_pair((xx[1].first - 1), 0);
  {
    var i = 1;
    while ((i <= (2 * n)))
    {
      if ((xx[i].first != xx[(i - 1)].first))
      {
        v += 1;
      }
      realx[v] = xx[i].first;
      var t = xx[i].second;
      if ((t > n))
      {
        xend[(t - n)] = v;
      } else
      {
        xbegin[t] = v;
      }
      i += 1;
    }
  }
}

func compressY()
{
  {
    var i = 1;
    while ((i <= n))
    {
      yy[i] = make_pair(ybegin[i], i);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      yy[(i + n)] = make_pair(yend[i], (i + n));
      i += 1;
    }
  }
  sort((yy + 1), ((yy + (2 * n)) + 1));
  var v = 1;
  yy[0] = make_pair((yy[1].first - 1), 0);
  {
    var i = 1;
    while ((i <= (2 * n)))
    {
      if ((yy[i].first != yy[(i - 1)].first))
      {
        v += 1;
      }
      realy[v] = yy[i].first;
      var t = yy[i].second;
      if ((t > n))
      {
        yend[(t - n)] = v;
      } else
      {
        ybegin[t] = v;
      }
      i += 1;
    }
  }
}

func init()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(xbegin[i], ybegin[i], xend[i], yend[i]);
      i += 1;
    }
  }
  compressX();
  compressY();
}

func make_events()
{
  v = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((xbegin[i] == xend[i]))
      {
        event_vertical(xbegin[i], ybegin[i], yend[i]);
      } else
      {
        event_horizantal_begin(xbegin[i], ybegin[i], xend[i]);
        event_horizantal_end(xend[i], ybegin[i]);
      }
      i += 1;
    }
  }
  sort((events + 1), ((events + v) + 1), CMP);
}

func findsum(x: dynamic)
{
  var s = 0;
  while ((x >= 1))
  {
    s += fen[x];
    x = ((x & ((x - 1))));
  }
  return s;
}

func md(x: dynamic, y: dynamic)
{
  while ((x <= 400010))
  {
    fen[x] += y;
    x = (((x | ((x - 1)))) + 1);
  }
}

func findset(x: dynamic)
{
  if ((parent[x] == 0))
  {
    return x;
  }
  parent[x] = findset(parent[x]);
  return parent[x];
}

func divide(y: dynamic, l: dynamic)
{
  var it2 = s.upper_bound(y);
  it2 -= 1;
  var cnt = lefts[l];
  if ((y > l))
  {
    var r = (*it2);
    var nsize = (findsum(r) - findsum((l - 1)));
    lefts[l].right = r;
    lefts[l].number = nsize;
  } else
  {
    lefts.erase(l);
  }
  if ((y < cnt.right))
  {
    it2 += 1;
    var nl = (*it2);
    var nsize = (findsum(cnt.right) - findsum((nl - 1)));
    lefts[nl].right = cnt.right;
    lefts[nl].dsu = cnt.dsu;
    lefts[nl].number = nsize;
  }
}

func main()
{
  ios_base.sync_with_stdio(0);
  init();
  make_events();
  compcount = 0;
  best = 0;
  s.insert(0);
  s.insert(2000000);
  lefts[2000000] = [2000000, 0, 0];
  lefts[0] = [0, 0, 0];
  {
    var i = 1;
    while ((i <= v))
    {
      if ((events[i].type_cpp == 1))
      {
        var y = events[i].y;
        compcount += 1;
        ans[compcount] = (realx[events[i].righty] - realx[events[i].x]);
        var it1 = lefts.upper_bound(y);
        it1 -= 1;
        var l = it1->first;
        if (((l <= y) && (y <= lefts[l].right)))
        {
          divide(y, l);
        }
        s.insert(y);
        lefts[y].right = y;
        lefts[y].dsu = compcount;
        lefts[y].number = 1;
        md(y, 1);
      }
      if ((events[i].type_cpp == 3))
      {
        var y = events[i].y;
        s.erase(y);
        var it = lefts.upper_bound(y);
        it -= 1;
        md(y, -1);
        var l = it->first;
        if ((lefts[l].right == l))
        {
          lefts.erase(l);
        } else if ((lefts[l].right == y))
        {
          var it = s.lower_bound(y);
          it -= 1;
          lefts[l].right = (*it);
          lefts[l].number -= 1;
        } else if ((l == y))
        {
          var it = s.lower_bound(y);
          var cnt = lefts[l];
          lefts.erase(l);
          lefts[(*it)] = cnt;
          lefts[(*it)].number -= 1;
        } else
        {
          lefts[l].number -= 1;
        }
      }
      if ((events[i].type_cpp == 2))
      {
        var yl = events[i].y;
        var yr = events[i].righty;
        if (((realy[yr] - realy[yl]) > best))
        {
          best = (realy[yr] - realy[yl]);
        }
        if (((findsum(yr) - findsum((yl - 1))) == 0))
        {
          i += 1;
          continue;
        }
        var nDSU = 0;
        var nl = 0;
        var nr = 0;
        var dif = 1;
        var it = lefts.lower_bound(yl);
        it -= 1;
        var l = it->first;
        var cnt = lefts[l];
        if (((cnt.right >= yl) && (l <= yl)))
        {
          nl = l;
          nr = cnt.right;
          nDSU = findset(cnt.dsu);
          lefts.erase(l);
        }
        while (true)
        {
          var it1 = lefts.lower_bound(yl);
          var l = it1->first;
          if ((l > yr))
          {
            break;
          }
          if ((lefts[l].right > yr))
          {
            break;
          }
          if ((nl == 0))
          {
            nl = l;
          }
          var cnt = lefts[l];
          nr = cnt.right;
          if ((nDSU == 0))
          {
            nDSU = findset(cnt.dsu);
          } else if ((nDSU != findset(cnt.dsu)))
          {
            ans[nDSU] += ans[findset(cnt.dsu)];
            parent[findset(cnt.dsu)] = nDSU;
            dif += 1;
          }
          lefts.erase(l);
        }
        it = lefts.upper_bound(yr);
        it -= 1;
        l = it->first;
        cnt = lefts[l];
        if (((cnt.right >= yr) && (l <= yr)))
        {
          if ((nl == 0))
          {
            nl = l;
          }
          nr = cnt.right;
          if ((nDSU == 0))
          {
            nDSU = findset(cnt.dsu);
          } else if ((nDSU != findset(cnt.dsu)))
          {
            ans[nDSU] += ans[findset(cnt.dsu)];
            parent[findset(cnt.dsu)] = nDSU;
            dif += 1;
          }
          lefts.erase(l);
        }
        if ((nDSU != 0))
        {
          lefts[nl].right = nr;
          lefts[nl].dsu = nDSU;
          lefts[nl].number = (findsum(nr) - findsum((nl - 1)));
          ans[nDSU] += ((((realy[yr] - realy[yl]) - (((findsum(yr) - findsum((yl - 1))) - 1))) + dif) - 1);
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= compcount))
    {
      if ((ans[i] > best))
      {
        best = ans[i];
      }
      i += 1;
    }
  }
  write(best, "\n");
  return 0;
}
