// Translated from solution.cpp.

var EPS = 1e-9;

var PI = acos(-1.0);

var INF = (2e+9 + 3);

func REP(i: dynamic, n: dynamic)
{
  cpp_macro("for (int i = 0; i < (int)(n); i++)");
}

func FOR(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for (int i = (s); i < (int)(n); i++)");
}

func FOREQ(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for (int i = (s); i <= (int)(n); i++)");
}

func FORIT(it: dynamic, c: dynamic)
{
  cpp_macro("for (__typeof((c).begin())it = (c).begin(); it != (c).end(); it++)");
}

func MEMSET(v: dynamic, h: dynamic)
{
  return cpp_expression("// // Problem: Kimagagure");
}

class Node
{
  var v: dynamic;
  var sum: dynamic;
  func Node()
  {
      this->v = cpp_construct(0);
      this->sum = cpp_construct(0);
    }
  func Node(v: dynamic)
  {
      this->v = cpp_construct(v);
      this->sum = cpp_construct(0);
    }
}

func Merge(left: dynamic, right: dynamic)
{
  return Node(max((left.v + left.sum), (right.v + right.sum)));
}

class SegmentTree
{
  var MAX_DEPTH: dynamic;
  var SIZE: dynamic;
  var updated: dynamic = cpp_array(SIZE);
  var data: dynamic = cpp_array(SIZE);
  func SegmentTree()
  {
      memset(updated, false, cpp_sizeof((updated)));
      MEMSET(data, 0);
    }
  func change(left: dynamic, right: dynamic, v: dynamic)
  {
      assert((left <= right));
      return in_set(v, 0, 1, left, right);
    }
  func get(left: dynamic, right: dynamic)
  {
      assert((left <= right));
      var node = in_get(0, 1, left, right);
      return (node.v + node.sum);
    }
  func Divide(node: dynamic)
  {
      if (((!updated[node]) || (node >= ((1 << MAX_DEPTH)))))
      {
        return;
      }
      updated[node] = false;
      updated[(node * 2)] = true;
      updated[((node * 2) + 1)] = true;
      data[(node * 2)].sum += data[node].sum;
      data[((node * 2) + 1)].sum += data[node].sum;
      data[node].v += data[node].sum;
      data[node].sum = 0;
    }
  func in_set(v: dynamic, depth: dynamic, node: dynamic, left: dynamic, right: dynamic)
  {
      var width = (1 << ((MAX_DEPTH - depth)));
      var index = (node - ((1 << depth)));
      var node_left = (index * width);
      var node_mid = (node_left + ((width >> 1)));
      Divide(node);
      if (((((right - left) + 1) == width) && (left == node_left)))
      {
        updated[node] = true;
        data[node].sum += v;
      } else
      {
        if ((right < node_mid))
        {
          in_set(v, (depth + 1), (node * 2), left, right);
        } else if ((left >= node_mid))
        {
          in_set(v, (depth + 1), ((node * 2) + 1), left, right);
        } else
        {
          in_set(v, (depth + 1), (node * 2), left, (node_mid - 1));
          in_set(v, (depth + 1), ((node * 2) + 1), node_mid, right);
        }
        data[node] = Merge(data[(node * 2)], data[((node * 2) + 1)]);
      }
    }
  func in_get(depth: dynamic, node: dynamic, left: dynamic, right: dynamic)
  {
      var width = (1 << ((MAX_DEPTH - depth)));
      var index = (node - ((1 << depth)));
      var node_left = (index * width);
      var node_mid = (node_left + ((width >> 1)));
      Divide(node);
      if (((((right - left) + 1) == width) && (left == node_left)))
      {
        return data[node];
      } else if ((right < node_mid))
      {
        return in_get((depth + 1), (node * 2), left, right);
      } else if ((left >= node_mid))
      {
        return in_get((depth + 1), ((node * 2) + 1), left, right);
      }
      return Merge(in_get((depth + 1), (node * 2), left, (node_mid - 1)), in_get((depth + 1), ((node * 2) + 1), node_mid, right));
    }
}

class Rect
{
  var dirs: dynamic;
  var initial_dir: dynamic;
  var dir: dynamic;
  var x1: dynamic;
  var y1: dynamic;
  var x2: dynamic;
  var y2: dynamic;
  func Rect()
  {
      this->dirs = cpp_construct(0);
      this->initial_dir = cpp_construct(0);
    }
  func Rect(dir: dynamic, x1: dynamic, y1: dynamic, x2: dynamic, y2: dynamic)
  {
      this->dirs = cpp_construct(0);
      this->initial_dir = cpp_construct(dir);
      this->dir = cpp_construct(dir);
      this->x1 = cpp_construct(x1);
      this->y1 = cpp_construct(y1);
      this->x2 = cpp_construct(x2);
      this->y2 = cpp_construct(y2);
    }
  func Move(d: dynamic, lower: dynamic, upper: dynamic, index: dynamic)
  {
      assert((dir != -1));
      assert(((d == 1) || (d == -1)));
      if ((d == 1))
      {
        dirs |= (1 << index);
      }
      dir = ((((dir + d) + 4)) % 4);
      if ((dir == 0))
      {
        Expand(lower, 0, upper, 0);
      }
      if ((dir == 1))
      {
        Expand(0, lower, 0, upper);
      }
      if ((dir == 2))
      {
        Expand((-upper), 0, (-lower), 0);
      }
      if ((dir == 3))
      {
        Expand(0, (-upper), 0, (-lower));
      }
    }
  func Move2(pm: dynamic, lower: dynamic, upper: dynamic, index: dynamic)
  {
      assert((dir != -1));
      assert(((pm == 0) || (pm == 1)));
      if ((pm == 1))
      {
        dirs |= (1 << index);
      }
      var ds = [-1, 1];
      REP(i, 2);
      {
        var ndir = ((((dir + ds[i]) + 4)) % 4);
        if (((((pm == 0) && (ndir >= 2))) || (((pm == 1) && (ndir <= 1)))))
        {
          dir = ndir;
          break;
        }
      }
      if ((dir == 0))
      {
        Expand(lower, 0, upper, 0);
      }
      if ((dir == 1))
      {
        Expand(0, lower, 0, upper);
      }
      if ((dir == 2))
      {
        Expand((-upper), 0, (-lower), 0);
      }
      if ((dir == 3))
      {
        Expand(0, (-upper), 0, (-lower));
      }
    }
  func Expand(lx: dynamic, ly: dynamic, ux: dynamic, uy: dynamic)
  {
      x1 += lx;
      y1 += ly;
      x2 += ux;
      y2 += uy;
    }
}

func Hit(r1: dynamic, r2: dynamic)
{
  return ((((r1.x1 <= r2.x2) && (r2.x1 <= r1.x2)) && (r1.y1 <= r2.y2)) && (r2.y1 <= r1.y2));
}

func operator_shift_left(os: dynamic, rhs: dynamic)
{
  (((((((((os << "(") << rhs.x1) << ", ") << rhs.y1) << ", ") << rhs.x2) << ", ") << rhs.y2) << ")");
  return os;
}

class Event
{
  var index: dynamic;
  var inout: dynamic;
  var x: dynamic;
  var y1: dynamic;
  var y2: dynamic;
  func Event(index: dynamic, inout: dynamic, x: dynamic, y1: dynamic, y2: dynamic)
  {
      this->index = cpp_construct(index);
      this->inout = cpp_construct(inout);
      this->x = cpp_construct(x);
      this->y1 = cpp_construct(y1);
      this->y2 = cpp_construct(y2);
    }
  func operator_less(rhs: dynamic)
  {
      if ((x != rhs.x))
      {
        return (x < rhs.x);
      }
      return (inout < rhs.inout);
    }
}

func Mirror(rect: dynamic, init_dir: dynamic, X: dynamic, Y: dynamic)
{
  REP(i, rect.size());
  {
    var r = rect[i];
    var rev = r;
    rev.x1 = (X - r.x2);
    rev.y1 = (Y - r.y2);
    rev.x2 = (X - r.x1);
    rev.y2 = (Y - r.y1);
    rev.dir = rev.initial_dir;
    rect[i] = rev;
  }
}

var n: dynamic;

var X: dynamic;

var Y: dynamic;

var dirs = cpp_array(100);

var lower = cpp_array(100);

var upper = cpp_array(100);

var ans_dirs = cpp_array(100);

var ans_l = cpp_array(100);

func simulate(rects: dynamic, dir: dynamic, l: dynamic, u: dynamic, index: dynamic)
{
  var cnt = 0;
  var ret: dynamic;
  var ds: dynamic;
  if ((dir != 0))
  {
    ds.push_back(dir);
    ret.resize(rects.size());
  } else
  {
    ds.push_back(1);
    ds.push_back(-1);
    ret.resize((rects.size() * 2));
  }
  return ret;
}

var stree: dynamic;

func IntersectRect(rs1: dynamic, rs2: dynamic, swapxy: dynamic)
{
  if (((rs1.size() == 0) || (rs2.size() == 0)))
  {
    return -1;
  }
  var rss = [(&rs1), (&rs2)];
  {
    var ys: dynamic;
    REP(iter, 2);
    {
      FORIT(it, (*rss[iter]));
      {
        if (swapxy)
        {
          swap(it->x1, it->y1);
          swap(it->x2, it->y2);
        }
        ys[it->y1] = 0;
        ys[it->y2] = 0;
      }
    }
    var index = 0;
    REP(iter, 2);
    {
      FORIT(it, (*rss[iter]));
      {
        it->y1 = ys[it->y1];
        it->y2 = ys[it->y2];
      }
    }
  }
  REP(iter, 2);
  {
    stree = SegmentTree();
    var events: dynamic;
    FORIT(it, (*rss[0]));
    {
      events.push_back(Event(-1, 1, it->x1, it->y1, it->y2));
      events.push_back(Event(-1, -1, (it->x2 + 1), it->y1, it->y2));
    }
    var cnt = 0;
    FORIT(it, (*rss[1]));
    {
      events.push_back(Event(cnt, 2, it->x1, it->y1, it->y2));
      events.push_back(Event(cnt, 2, it->x2, it->y1, it->y2));
      cnt += 1;
    }
    sort(events.begin(), events.end());
    swap(rss[0], rss[1]);
  }
  return -1;
}

func IntersectRect2(rs1: dynamic, rs2: dynamic, swapxy: dynamic)
{
  if (((rs1.size() == 0) || (rs2.size() == 0)))
  {
    return -1;
  }
  var rss = [(&rs1), (&rs2)];
  {
    REP(iter, 2);
    {
      FORIT(it, (*rss[iter]));
      {
        if (swapxy)
        {
          swap(it->x1, it->y1);
          swap(it->x2, it->y2);
        }
      }
    }
  }
  REP(iter, 2);
  {
    var events: dynamic;
    FORIT(it, (*rss[0]));
    {
      events.push_back(Event(-1, 1, it->x1, it->y1, it->y2));
      events.push_back(Event(-1, -1, (it->x2 + 1), it->y1, it->y2));
    }
    var cnt = 0;
    FORIT(it, (*rss[1]));
    {
      events.push_back(Event(cnt, 2, it->x1, it->y1, it->y2));
      events.push_back(Event(cnt, 2, it->x2, it->y1, it->y2));
      cnt += 1;
    }
    sort(events.begin(), events.end());
    var hit = 0;
    swap(rss[0], rss[1]);
  }
  return -1;
}

func GetSolvingDir(depth: dynamic, xy: dynamic, flags: dynamic)
{
  if ((dirs[depth] != 0))
  {
    return -999;
  }
  if (((dirs[depth] == 0) && (dirs[(depth + 1)] == 0)))
  {
    if ((xy == (depth % 2)))
    {
      return -1;
    }
    return 0;
  }
  return (((flags >> depth)) & 1);
}

func simulate2(rects: dynamic, pm: dynamic, l: dynamic, u: dynamic, index: dynamic)
{
  var cnt = 0;
  var ret: dynamic;
  var pms: dynamic;
  if ((pm >= 0))
  {
    pms.push_back(pm);
    ret.resize(rects.size());
  } else
  {
    pms.push_back(0);
    pms.push_back(1);
    ret.resize((rects.size() * 2));
  }
  return ret;
}

func Solve(flags: dynamic)
{
  var ans_flags = [-1, -1];
  REP(xy, 2);
  {
    var center = n;
    var rect1: dynamic;
    rect1.push_back(Rect(0, 0, 0, 0, 0));
    var rect2: dynamic;
    var left_upper = (center % 2);
    rect2.push_back(Rect(left_upper, 0, 0, 0, 0));
    rect2.push_back(Rect((left_upper + 2), 0, 0, 0, 0));
    var lx = if ((xy == 0)) 0 else X;
    var ly = if ((xy == 0)) Y else 0;
    Mirror(rect2, left_upper, lx, ly);
    var rs1 = cpp_array(4);
    var rs2 = cpp_array(4);
    if ((((center != n) && (dirs[center] == 0)) && (dirs[(center + 1)] != 0)))
    {
    } else
    {
    }
    var ans_dir_flags = -1;
    REP(dir, 4);
    {
      ans_dir_flags = IntersectRect2(rs1[dir], rs2[dir], (xy ^ 1));
      if ((ans_dir_flags != -1))
      {
        break;
      }
    }
    if ((ans_dir_flags == -1))
    {
      return -1;
    }
    ans_flags[xy] = ans_dir_flags;
  }
  var dir = 0;
  return 1;
}

func Dfs(depth: dynamic, flags: dynamic)
{
  if ((depth == n))
  {
    return Solve(flags);
  }
  if (((dirs[depth] == 0) && (dirs[(depth + 1)] != 0)))
  {
    assert((dirs[depth] == 0));
    REP(iter, 2);
    {
      var nflags = (flags | ((cpp_cast(iter) << depth)));
      if ((Dfs((depth + 1), nflags) != -1))
      {
        return 1;
      }
    }
    return -1;
  }
  return Dfs((depth + 1), flags);
}

func RestoreDistance()
{
  var r = cpp_construct(0, 0, 0, 0, 0);
  {
    var rects = cpp_construct(1, r);
    var rect = rects[0];
    assert(Hit(Rect(0, X, Y, X, Y), rect));
  }
}

func Check()
{
  var rect = cpp_construct(1, Rect(0, 0, 0, 0, 0));
  if (((rect[0].x1 != X) || (rect[0].y1 != Y)))
  {
    return false;
  }
  return true;
}

func main()
{
  while ((scanf("%d %lld %lld", (&n), (&X), (&Y)) > 0))
  {
    var center = n;
    var div = 0;
    dirs[n] = 0;
    var segment = 0;
    var ans_dir = -1;
    if ((segment > ((n / 4) + 1)))
    {
      var rect1: dynamic;
      rect1.push_back(Rect(0, 0, 0, 0, 0));
      var rect2: dynamic;
      var left_upper = (center % 2);
      rect2.push_back(Rect(left_upper, 0, 0, 0, 0));
      rect2.push_back(Rect((left_upper + 2), 0, 0, 0, 0));
      Mirror(rect2, left_upper, X, Y);
      var rs1 = cpp_array(4);
      var rs2 = cpp_array(4);
      REP(dir, 4);
      {
        ans_dir = IntersectRect(rs1[dir], rs2[dir], false);
        if ((ans_dir != -1))
        {
          break;
        }
      }
    } else
    {
      ans_dir = Dfs(0, 0);
    }
    if ((ans_dir == -1))
    {
      puts("-1");
      cpp_goto("goto next;");
    }
    RestoreDistance();
    printf("%d\n", n);
    assert(Check());
  }
}

func FORIT(argument_0: dynamic, argument_1: dynamic)
{
      var r = (*it2);
      r.Move(d, l, u, index);
      ret[cpp_update(cnt, "++")] = r;
    }

func FORIT(argument_0: dynamic, argument_1: dynamic)
{
    var d = (*it1);
  }

func FORIT(argument_0: dynamic, argument_1: dynamic)
{
      it->second = cpp_update(index, "++");
    }

func FORIT(argument_0: dynamic, argument_1: dynamic)
{
      var e = (*it);
      if ((e.index == -1))
      {
        stree.change(e.y1, e.y2, e.inout);
      } else
      {
        if ((stree.get(e.y1, e.y2) > 0))
        {
          var rect2 = ((*rss[1]))[e.index];
          REP(i, rss[0]->size());
          {
            if (Hit(((*rss[0]))[i], rect2))
            {
              return (((*rss[0]))[i].dirs | rect2.dirs);
            }
          }
          assert(false);
        }
      }
    }

func FORIT(argument_0: dynamic, argument_1: dynamic)
{
      var e = (*it);
      if ((e.index == -1))
      {
        hit += e.inout;
        assert((hit >= 0));
      } else
      {
        if ((hit > 0))
        {
          var rect2 = ((*rss[1]))[e.index];
          REP(i, rss[0]->size());
          {
            if (Hit(((*rss[0]))[i], rect2))
            {
              return (((*rss[0]))[i].dirs | rect2.dirs);
            }
          }
          assert(false);
        }
      }
    }

func FORIT(argument_0: dynamic, argument_1: dynamic)
{
      var r = (*it2);
      r.Move2(v, l, u, index);
      ret[cpp_update(cnt, "++")] = r;
    }

func FORIT(argument_0: dynamic, argument_1: dynamic)
{
    var v = (*it1);
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      var pm = GetSolvingDir(i, xy, flags);
      var l = lower[i];
      var u = upper[i];
      if ((xy != (i % 2)))
      {
        l = 0;
        u = 0;
      }
      if ((dirs[i] != 0))
      {
        rect1 = simulate(rect1, dirs[i], l, u, i);
      } else
      {
        rect1 = simulate2(rect1, pm, l, u, i);
      }
      if ((rect1.size() > ((1 << 7))))
      {
        center = (i + 1);
        break;
      }
    }

func FOR(argument_0: dynamic, argument_1: dynamic, argument_2: dynamic)
{
      var pm = GetSolvingDir(i, xy, flags);
      var l = lower[i];
      var u = upper[i];
      if ((xy != (i % 2)))
      {
        l = 0;
        u = 0;
      }
      if ((dirs[i] != 0))
      {
        rect2 = simulate(rect2, dirs[i], l, u, i);
      } else
      {
        rect2 = simulate2(rect2, pm, l, u, i);
      }
    }

func FORIT(argument_0: dynamic, argument_1: dynamic)
{
        rs1[0].push_back((*it));
      }

func FORIT(argument_0: dynamic, argument_1: dynamic)
{
        rs2[0].push_back((*it));
      }

func FORIT(argument_0: dynamic, argument_1: dynamic)
{
        rs1[it->dir].push_back((*it));
      }

func FORIT(argument_0: dynamic, argument_1: dynamic)
{
        rs2[it->dir].push_back((*it));
      }

func REP(argument_0: dynamic, argument_1: dynamic)
{
    if ((dirs[depth] != 0))
    {
      ans_dirs[depth] = dirs[depth];
    } else
    {
      var xy = (depth % 2);
      var v = (((ans_flags[xy] >> depth)) & 1);
      var ds = [-1, 1];
      REP(i, 2);
      {
        var ndir = ((((dir + ds[i]) + 4)) % 4);
        if (((((v == 0) && (ndir >= 2))) || (((v == 1) && (ndir <= 1)))))
        {
          ans_dirs[depth] = ds[i];
        }
      }
    }
    dir = ((((dir + ans_dirs[depth]) + 4)) % 4);
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      rects = simulate(rects, ans_dirs[i], lower[i], upper[i], i);
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
    var ndir = ((((r.dir + ans_dirs[i]) + 4)) % 4);
    var l = lower[i];
    var u = upper[i];
    while ((l != u))
    {
      var m = (((l + u)) / 2);
      assert((l < u));
      var rects = cpp_construct(1, r);
      rects = simulate(rects, ans_dirs[i], m, m, i);
      FOR(j, (i + 1), n);
      {
        rects = simulate(rects, ans_dirs[j], lower[j], upper[j], j);
      }
      var rect = rects[0];
      if (((((((ndir == 0) && (rect.x2 < X))) || (((ndir == 1) && (rect.y2 < Y)))) || (((ndir == 2) && (X < rect.x1)))) || (((ndir == 3) && (Y < rect.y1)))))
      {
        l = (m + 1);
      } else
      {
        u = m;
      }
    }
    ans_l[i] = l;
    r.Move(ans_dirs[i], l, l, i);
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
    if (((ans_l[i] < lower[i]) || (upper[i] < ans_l[i])))
    {
      return false;
    }
    if (((dirs[i] != 0) && (dirs[i] != ans_dirs[i])))
    {
      return false;
    }
    rect = simulate(rect, ans_dirs[i], ans_l[i], ans_l[i], i);
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      var c: dynamic;
      var v = scanf(" %c %lld %lld", (&c), (&lower[i]), (&upper[i]));
      assert((v == 3));
      if ((c == cpp_char("L")))
      {
        dirs[i] = 1;
      }
      if ((c == cpp_char("?")))
      {
        dirs[i] = 0;
      }
      if ((c == cpp_char("R")))
      {
        dirs[i] = -1;
      }
      if ((dirs[i] == 0))
      {
        div += 1;
        if ((div == 21))
        {
          center = i;
        }
      }
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      if (((dirs[i] == 0) && (dirs[(i + 1)] != 0)))
      {
        segment += 1;
      }
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
        rect1 = simulate(rect1, dirs[i], lower[i], upper[i], i);
      }

func FOR(argument_0: dynamic, argument_1: dynamic, argument_2: dynamic)
{
        rect2 = simulate(rect2, dirs[i], lower[i], upper[i], i);
      }

func FORIT(argument_0: dynamic, argument_1: dynamic)
{
        rs1[it->dir].push_back((*it));
      }

func FORIT(argument_0: dynamic, argument_1: dynamic)
{
        rs2[it->dir].push_back((*it));
      }

func REP(argument_0: dynamic, argument_1: dynamic)
{
        ans_dirs[i] = if (((((ans_dir >> i)) & 1))) 1 else -1;
      }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      printf("%c %lld\n", if ((ans_dirs[i] == 1)) cpp_char("L") else cpp_char("R"), ans_l[i]);
    }
