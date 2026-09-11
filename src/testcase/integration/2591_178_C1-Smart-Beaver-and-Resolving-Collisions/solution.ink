// Translated from solution.cpp.

class Node
{
  var fr: dynamic = cpp_uninitialized();
  var lb: dynamic = cpp_uninitialized();
  var rb: dynamic = cpp_uninitialized();
  var lc: dynamic = cpp_uninitialized();
  var rc: dynamic = cpp_uninitialized();
}

func buildSeg(p: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  p = cpp_new();
  p->fr = true;
  p->lb = l;
  p->rb = r;
  if ((p->lb < p->rb))
  {
    var mid: dynamic = (((p->lb + p->rb)) / 2);
    buildSeg(p->lc, p->lb, mid);
    buildSeg(p->rc, (mid + 1), p->rb);
  }
}

func setVal(p: dynamic, x: dynamic, val: dynamic) -> dynamic
{
  if ((p->lb == p->rb))
  {
    p->fr = val;
  } else
  {
    var mid: dynamic = (((p->lb + p->rb)) / 2);
    if ((x <= mid))
    {
      setVal(p->lc, x, val);
    } else
    {
      setVal(p->rc, x, val);
    }
    p->fr = (p->lc->fr | p->rc->fr);
  }
}

func findFr(p: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if (p->fr)
  {
    if ((p->lb == p->rb))
    {
      return p->lb;
    } else
    {
      var mid: dynamic = (((p->lb + p->rb)) / 2);
      var ret: dynamic = -1;
      if ((l <= mid))
      {
        ret = findFr(p->lc, l, r);
      }
      if (((((mid + 1) <= r)) && ((ret == -1))))
      {
        ret = findFr(p->rc, l, r);
      }
      return ret;
    }
  }
  return -1;
}

var N: dynamic = 200010;

var h: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var ptN: dynamic = cpp_uninitialized();

var pt: dynamic = cpp_array(N);

var cost: dynamic = cpp_uninitialized();

var inP: dynamic = cpp_array(N);

var rt: dynamic = cpp_uninitialized();

var ps: dynamic = cpp_uninitialized();

func initSeg() -> dynamic
{
  ptN = 0;
  var cnt: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < h))
    {
      inP[i].first = -1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < h))
    {
      if ((inP[i].first == -1))
      {
        pt[ptN] = cnt;
        {
          var j: dynamic = i;
          while ((inP[j].first == -1))
          {
            inP[j] = pair(ptN, cpp_update(cnt, "++"));
            j = (((j + m)) % h);
          }
        }
        ptN += 1;
      }
      i += 1;
    }
  }
  pt[ptN] = cnt;
  buildSeg(rt, 0, (cnt - 1));
}

func pushHash(i: dynamic, v: dynamic) -> dynamic
{
  var p: dynamic = findFr(rt, inP[v].second, (pt[(inP[v].first + 1)] - 1));
  if ((p == -1))
  {
    p = findFr(rt, pt[inP[v].first], (inP[v].second - 1));
  }
  setVal(rt, p, false);
  ps.insert(pair(i, p));
  cost +=  (((p >= inP[v].second))) ? ((p - inP[v].second)) : ((((pt[(inP[v].first + 1)] - inP[v].second) + p) - pt[inP[v].first]));
}

func popHash(i: dynamic) -> dynamic
{
  var x: dynamic = ps.find(i);
  setVal(rt, x->second, true);
  ps.erase(x);
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  read(h, m, n);
  initSeg();
  cost = 0;
  {
    var k: dynamic = 0;
    while ((k < n))
    {
      var c: dynamic = cpp_uninitialized();
      var i: dynamic = cpp_uninitialized();
      read(c, i);
      if ((c == cpp_char("+")))
      {
        var v: dynamic = cpp_uninitialized();
        read(v);
        pushHash(i, v);
      } else
      {
        popHash(i);
      }
      k += 1;
    }
  }
  write(cost, "\n");
  return 0;
}
