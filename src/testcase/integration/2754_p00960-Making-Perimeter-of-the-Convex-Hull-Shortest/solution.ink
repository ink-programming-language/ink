// Translated from solution.cpp.

class PT
{
  var x: dynamic;
  var y: dynamic;
  func PT(x: dynamic = 0, y: dynamic = 0)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
  func in_cpp()
  {
      scanf("%d%d", (&x), (&y));
    }
  func operator_less(pts: dynamic)
  {
      return (make_pair(x, y) < make_pair(pts.x, pts.y));
    }
  func operator_equal(pts: dynamic)
  {
      return (make_pair(x, y) == make_pair(pts.x, pts.y));
    }
}

func sqr(x: dynamic)
{
  return (cpp_cast(x) * x);
}

func dis(p1: dynamic, p2: dynamic)
{
  return sqrt((sqr((p1.x - p2.x)) + sqr((p1.y - p2.y))));
}

func vect(p: dynamic, p1: dynamic, p2: dynamic)
{
  return (((1 * ((p1.x - p.x))) * ((p2.y - p.y))) - ((1 * ((p1.y - p.y))) * ((p2.x - p.x))));
}

func scal(p: dynamic, p1: dynamic, p2: dynamic)
{
  return (((1 * ((p1.x - p.x))) * ((p2.x - p.x))) + ((1 * ((p1.y - p.y))) * ((p2.y - p.y))));
}

func check(p1: dynamic, p2: dynamic, p3: dynamic)
{
  if ((vect(p1, p2, p3) < 0))
  {
    return 1;
  }
  if ((!vect(p1, p2, p3)))
  {
    return (scal(p2, p1, p3) <= 0);
  }
  return 0;
}

var pts: dynamic;

var cvx: dynamic;

func convex()
{
  sort(pts.begin(), pts.end());
  pts.erase(unique(pts.begin(), pts.end()), pts.end());
  if ((pts.size() == 1))
  {
    cvx.push_back(pts[0]);
    return;
  }
  {
    var times = 0;
    while ((times < 2))
    {
      for (var t in pts)
      {
        while (((cvx.size() > 1) && check(cvx[(cvx.size() - 2)], cvx.back(), t)))
        {
          cvx.pop_back();
        }
        cvx.push_back(t);
      }
      reverse(pts.begin(), pts.end());
      times += 1;
    }
  }
  cvx.pop_back();
}

func getId(p: dynamic)
{
  return (lower_bound(pts.begin(), pts.end(), p) - pts.begin());
}

func getConvex(lft: dynamic, cur: dynamic, rht: dynamic, nV: dynamic)
{
  var nC: dynamic;
  var px = getId(lft);
  var cx = getId(cur);
  var rx = getId(rht);
  var now = px;
  while ((now != cx))
  {
    if ((!((pts[now] == nV))))
    {
      while (((nC.size() > 1) && check(nC[(nC.size() - 2)], nC.back(), pts[now])))
      {
        nC.pop_back();
      }
      nC.push_back(pts[now]);
    }
    if ((now < cx))
    {
      now += 1;
    } else
    {
      now -= 1;
    }
  }
  while ((now != rx))
  {
    if ((now < rx))
    {
      now += 1;
    } else
    {
      now -= 1;
    }
    if ((!((pts[now] == nV))))
    {
      while (((nC.size() > 1) && check(nC[(nC.size() - 2)], nC.back(), pts[now])))
      {
        nC.pop_back();
      }
      nC.push_back(pts[now]);
    }
  }
  return nC;
}

var profit: dynamic;

var PM: dynamic;

var ans: dynamic;

func process()
{
  var sz = cvx.size();
  {
    var i = 0;
    while ((i < cvx.size()))
    {
      var lft = cvx[((((i - 1) + sz)) % sz)];
      var cur = cvx[i];
      var rht = cvx[(((i + 1)) % sz)];
      var lose = (dis(lft, cur) + dis(cur, rht));
      var nC = getConvex(lft, cur, rht, PT(1e8, 1e8));
      var get = 0;
      {
        var j = 1;
        while ((j < nC.size()))
        {
          get += dis(nC[(j - 1)], nC[j]);
          j += 1;
        }
      }
      profit.push_back((lose - get));
      PM.insert((lose - get));
      nC.push_back(cvx[(((i + 2)) % sz)]);
      {
        var j = 1;
        while ((j < (cpp_cast(nC.size()) - 1)))
        {
          var lft = nC[(j - 1)];
          var cur = nC[j];
          var nxt = nC[(j + 1)];
          var lose = (dis(lft, cur) + dis(cur, nxt));
          var nC = getConvex(lft, cur, nxt, cur);
          var get = 0;
          {
            var k = 1;
            while ((k < nC.size()))
            {
              get += dis(nC[(k - 1)], nC[k]);
              k += 1;
            }
          }
          ans = max(ans, (((lose - get) + lose) - get));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < profit.size()))
    {
      var l = profit[((((i - 1) + sz)) % sz)];
      var c = profit[i];
      var r = profit[(((i + 1)) % sz)];
      PM.erase(PM.find(l));
      PM.erase(PM.find(r));
      PM.erase(PM.find(c));
      if (PM.size())
      {
        ans = max(ans, (c + (*(PM.rbegin()))));
      } else
      {
        ans = max(ans, c);
      }
      PM.insert(l);
      PM.insert(r);
      PM.insert(c);
      i += 1;
    }
  }
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  var p: dynamic;
  while (cpp_update(n, "--"))
  {
    p.in_cpp();
    pts.push_back(p);
  }
  convex();
  process();
  printf("%.10lf\n", ans);
}
