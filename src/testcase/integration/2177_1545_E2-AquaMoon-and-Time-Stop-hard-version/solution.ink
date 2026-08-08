// Translated from solution.cpp.

var PB = cpp_expression("#include");

var MP = cpp_expression("#include");

func SZ(v: dynamic)
{
  return cpp_expression("#include <algorit");
}

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

func FORE(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(a);i<=(b);++i)");
}

func REPE(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <a");
}

func FORSZ(i: dynamic, a: dynamic, v: dynamic)
{
  return cpp_expression("#include <algo");
}

func REPSZ(i: dynamic, v: dynamic)
{
  return cpp_expression("#include <al");
}

var rnd = cpp_construct(cpp_cast(chrono.steady_clock.now().time_since_epoch().count()));

func gcd(a: dynamic, b: dynamic)
{
  return if ((b == 0)) a else gcd(b, (a % b));
}

class SplayTree
{
  var nodes: dynamic;
  func reset()
  {
      nodes.clear();
    }
  func apply(x: dynamic, lazy: dynamic)
  {
      nodes[x].item += lazy;
      nodes[x].sum += lazy;
      nodes[x].lazy += lazy;
    }
  func push(x: dynamic)
  {
      cpp_statement("REP(z, 2)");
      if ((nodes[x].ch[z] != -1))
      {
        apply(nodes[x].ch[z], nodes[x].lazy);
      }
      nodes[x].lazy = Lazy();
    }
  func update(x: dynamic)
  {
      nodes[x].sum = Sum();
      if ((nodes[x].ch[0] != -1))
      {
        nodes[x].sum += nodes[nodes[x].ch[0]].sum;
      }
      nodes[x].sum += Sum(nodes[x].item);
      if ((nodes[x].ch[1] != -1))
      {
        nodes[x].sum += nodes[nodes[x].ch[1]].sum;
      }
    }
  func connect(x: dynamic, p: dynamic, z: dynamic)
  {
      if ((x != -1))
      {
        nodes[x].par = p;
      }
      if ((p != -1))
      {
        nodes[p].ch[z] = x;
      }
    }
  func disconnect(p: dynamic, z: dynamic)
  {
      var x = nodes[p].ch[z];
      nodes[p].ch[z] = -1;
      if ((x != -1))
      {
        nodes[x].par = -1;
      }
      return x;
    }
  func rotate(x: dynamic)
  {
      var p = nodes[x].par;
      var g = nodes[p].par;
      var z = if ((nodes[p].ch[0] == x)) 0 else 1;
      var y = nodes[x].ch[(1 - z)];
      push(p);
      push(x);
      connect(y, p, z);
      connect(p, x, (1 - z));
      connect(x, g, if ((g == -1)) -1 else if ((nodes[g].ch[0] == p)) 0 else 1);
      update(p);
    }
  func splay(x: dynamic, y: dynamic = -1)
  {
      if ((nodes[x].par == y))
      {
        return;
      }
      while ((nodes[x].par != y))
      {
        var p = nodes[x].par;
        var g = nodes[p].par;
        if ((g != y))
        {
          rotate(if ((((nodes[p].ch[0] == x)) == ((nodes[g].ch[0] == p)))) p else x);
        }
        rotate(x);
      }
      update(x);
    }
  func first(x: dynamic)
  {
      if ((x == -1))
      {
        return x;
      }
      splay(x);
      while ((nodes[x].ch[0] != -1))
      {
        x = nodes[x].ch[0];
      }
      splay(x);
      return x;
    }
  func last(x: dynamic)
  {
      if ((x == -1))
      {
        return x;
      }
      splay(x);
      while ((nodes[x].ch[1] != -1))
      {
        x = nodes[x].ch[1];
      }
      splay(x);
      return x;
    }
  func add(item: dynamic)
  {
      nodes.PB(Node(item, Sum(item), Lazy()));
      return (SZ(nodes) - 1);
    }
  func join(l: dynamic, r: dynamic)
  {
      if ((l == -1))
      {
        return r;
      }
      l = last(l);
      push(l);
      connect(r, l, 1);
      update(l);
      return l;
    }
  func split(x: dynamic, v: dynamic, l: dynamic, r: dynamic)
  {
      if ((x == -1))
      {
        l = cpp_assign(r, "=", -1);
        return;
      } else
      {
        splay(x);
      }
      l = cpp_assign(r, "=", -1);
      while ((x != -1))
      {
        push(x);
        if ((nodes[x].item.l() < v))
        {
          l = x;
          x = nodes[x].ch[1];
        } else
        {
          r = x;
          x = nodes[x].ch[0];
        }
      }
      if ((l != -1))
      {
        splay(l);
      }
      if ((r != -1))
      {
        splay(r, l);
      }
      if ((l == -1))
      {
        return;
      }
      assert((((nodes[l].par == -1) && (nodes[l].ch[1] == r)) && (((r == -1) || (nodes[r].ch[0] == -1)))));
      push(l);
      disconnect(l, 1);
      update(l);
      if ((nodes[l].item.r() < v))
      {
        return;
      }
      var splitted = nodes[l].item.split(v);
      if (((nodes[l].ch[0] != -1) && (nodes[nodes[l].ch[0]].item.r() == nodes[l].item.r())))
      {
        l = disconnect(l, 0);
      }
      if (((r == -1) || (splitted.l() != nodes[r].item.l())))
      {
        r = join(add(splitted), r);
      }
      update(l);
    }
  func gather(x: dynamic, ret: dynamic)
  {
      push(x);
      if ((nodes[x].ch[0] != -1))
      {
        gather(nodes[x].ch[0], ret);
      }
      ret.PB(nodes[x].item);
      if ((nodes[x].ch[1] != -1))
      {
        gather(nodes[x].ch[1], ret);
      }
    }
  func all(x: dynamic)
  {
      var ret: dynamic;
      if ((x != -1))
      {
        splay(x);
        gather(x, ret);
      }
      return ret;
    }
}

var MAXRECT = 200000;

var INF = 1000000000;

class Rect
{
  var lt: dynamic;
  var rt: dynamic;
  var lx: dynamic;
  var rx: dynamic;
}

var nrect: dynamic;

var sx: dynamic;

var rect = cpp_array(MAXRECT);

class Line
{
  var ly: dynamic;
  var ry: dynamic;
  var lcost: dynamic;
  var slope: dynamic;
  func Line()
  {
    }
  func Line(ly: dynamic, ry: dynamic, lcost: dynamic, slope: dynamic)
  {
      this->ly = cpp_construct(ly);
      this->ry = cpp_construct(ry);
      this->lcost = cpp_construct(lcost);
      this->slope = cpp_construct(if ((ly == ry)) 0 else slope);
    }
  func rcost()
  {
      return (lcost + (slope * ((ry - ly))));
    }
  func len()
  {
      return (ry - ly);
    }
  func setly(nly: dynamic)
  {
      lcost += (((nly - ly)) * slope);
      ly = nly;
      if ((ly == ry))
      {
        slope = 0;
      }
    }
  func setry(nry: dynamic)
  {
      ry = nry;
      if ((ly == ry))
      {
        slope = 0;
      }
    }
  func l()
  {
      return ly;
    }
  func r()
  {
      return ry;
    }
  func split(y: dynamic)
  {
      assert(((ly < y) && (y <= ry)));
      var ret = Line(y, ry, (lcost + (((y - ly)) * slope)), slope);
      setry((y - 1));
      return ret;
    }
}

class SumLine
{
  func SumLine()
  {
    }
  func SumLine(line: dynamic)
  {
    }
}

func operator_add_assign(a: dynamic, b: dynamic)
{
  return a;
}

class LazyLine
{
}

func operator_add_assign(a: dynamic, b: dynamic)
{
  return a;
}

func operator_add_assign(a: dynamic, b: dynamic)
{
  return a;
}

func operator_add_assign(a: dynamic, b: dynamic)
{
  return a;
}

var linetree: dynamic;

func printfunc(lineroot: dynamic, tline: dynamic)
{
  if ((lineroot == -1))
  {
    printf(" BLOCKED");
  } else
  {
    var alllines = linetree.all(lineroot);
    for (var line in alllines)
    {
      printf(" (%d,%d)..(%d,%d)", if ((line.ly == (-INF))) (-INF) else (line.ly + tline), if ((line.ly == (-INF))) INF else line.lcost, if ((line.ry == (+INF))) (+INF) else (line.ry + tline), if ((line.ry == (+INF))) INF else line.rcost());
    }
  }
  puts("");
}

var t: dynamic;

func rtrimfunc(node: dynamic, nry: dynamic)
{
  while (true)
  {
    node = linetree.last(node);
    if ((linetree.nodes[node].item.ry <= nry))
    {
      break;
    }
    if (((linetree.nodes[node].item.ly < nry) || ((linetree.nodes[node].item.ly == nry) && (linetree.nodes[node].ch[0] == -1))))
    {
      linetree.nodes[node].item.setry(nry);
      break;
    }
    node = linetree.disconnect(node, 0);
    assert((node != -1));
  }
  return node;
}

func lgrowfunc(node: dynamic, nly: dynamic)
{
  node = linetree.first(node);
  assert((nly <= linetree.nodes[node].item.ly));
  if ((nly == linetree.nodes[node].item.ly))
  {
    return node;
  }
  if (((linetree.nodes[node].item.slope == -1) || (linetree.nodes[node].item.ly == linetree.nodes[node].item.ry)))
  {
    linetree.nodes[node].item.slope = -1;
    linetree.nodes[node].item.setly(nly);
    linetree.update(node);
  } else
  {
    var line = cpp_construct(nly, linetree.nodes[node].item.ly, ((linetree.nodes[node].item.lcost + linetree.nodes[node].item.ly) - nly), -1);
    node = linetree.join(linetree.add(line), node);
  }
  return node;
}

func rgrowfunc(node: dynamic, nry: dynamic)
{
  node = linetree.last(node);
  assert((nry >= linetree.nodes[node].item.ry));
  if ((nry == linetree.nodes[node].item.ry))
  {
    return node;
  }
  if (((linetree.nodes[node].item.slope == +1) || (linetree.nodes[node].item.ly == linetree.nodes[node].item.ry)))
  {
    linetree.nodes[node].item.slope = +1;
    linetree.nodes[node].item.setry(nry);
    linetree.update(node);
  } else
  {
    var line = cpp_construct(linetree.nodes[node].item.ry, nry, linetree.nodes[node].item.rcost(), +1);
    node = linetree.join(node, linetree.add(line));
  }
  return node;
}

func shiftfunc(node: dynamic, dt: dynamic)
{
  if ((dt == 0))
  {
    return node;
  }
  node = linetree.first(node);
  var ply = linetree.nodes[node].item.ly;
  if ((ply != (-INF)))
  {
    node = lgrowfunc(node, (ply - dt));
  }
  node = linetree.last(node);
  var pry = linetree.nodes[node].item.ry;
  if ((pry != (+INF)))
  {
    node = rtrimfunc(node, (pry - dt));
  }
  return node;
}

func mergefunc(l: dynamic, r: dynamic)
{
  l = linetree.last(l);
  r = linetree.first(r);
  while ((((l != -1) && (r != -1)) && (linetree.nodes[l].item.rcost() > linetree.nodes[r].item.lcost)))
  {
    assert((linetree.nodes[l].item.ry == linetree.nodes[r].item.ly));
    if (((linetree.nodes[l].item.slope == -1) || (linetree.nodes[l].item.ly == linetree.nodes[l].item.ry)))
    {
      r = lgrowfunc(r, linetree.nodes[l].item.ly);
      l = linetree.disconnect(l, 0);
    } else
    {
      assert((linetree.nodes[l].item.slope == 1));
      if (((linetree.nodes[r].item.lcost + linetree.nodes[l].item.len()) <= linetree.nodes[l].item.lcost))
      {
        r = lgrowfunc(r, linetree.nodes[l].item.ly);
        l = linetree.disconnect(l, 0);
      } else
      {
        var y = (((linetree.nodes[r].item.lcost - linetree.nodes[l].item.lcost) + linetree.nodes[l].item.ly) + linetree.nodes[l].item.ry);
        assert(((y % 2) == 0));
        y /= 2;
        assert(((y > linetree.nodes[l].item.ly) && (y < linetree.nodes[l].item.ry)));
        r = lgrowfunc(r, y);
        linetree.nodes[l].item.setry(y);
      }
    }
    l = linetree.last(l);
    r = linetree.first(r);
  }
  while ((((l != -1) && (r != -1)) && (linetree.nodes[l].item.rcost() < linetree.nodes[r].item.lcost)))
  {
    assert((linetree.nodes[l].item.ry == linetree.nodes[r].item.ly));
    if (((linetree.nodes[r].item.slope == +1) || (linetree.nodes[r].item.ly == linetree.nodes[r].item.ry)))
    {
      l = rgrowfunc(l, linetree.nodes[r].item.ry);
      r = linetree.disconnect(r, 1);
    } else
    {
      assert((linetree.nodes[r].item.slope == -1));
      if (((linetree.nodes[l].item.rcost() + linetree.nodes[r].item.len()) <= linetree.nodes[r].item.rcost()))
      {
        l = rgrowfunc(l, linetree.nodes[r].item.ry);
        r = linetree.disconnect(r, 1);
      } else
      {
        var y = (((linetree.nodes[r].item.rcost() - linetree.nodes[l].item.rcost()) + linetree.nodes[r].item.ly) + linetree.nodes[r].item.ry);
        assert(((y % 2) == 0));
        y /= 2;
        assert(((y > linetree.nodes[r].item.ly) && (y < linetree.nodes[r].item.ry)));
        l = rgrowfunc(l, y);
        linetree.nodes[r].item.setly(y);
      }
    }
    l = linetree.last(l);
    r = linetree.first(r);
  }
  var ret = linetree.join(l, r);
  return ret;
}

class Region
{
  var lx: dynamic;
  var rx: dynamic;
  var blockcnt: dynamic;
  var lineroot: dynamic;
  var tline: dynamic;
  func Region(lx: dynamic, rx: dynamic, blockcnt: dynamic, lineroot: dynamic, tline: dynamic)
  {
      this->lx = cpp_construct(lx);
      this->rx = cpp_construct(rx);
      this->blockcnt = cpp_construct(blockcnt);
      this->lineroot = cpp_construct(lineroot);
      this->tline = cpp_construct(tline);
    }
  func l()
  {
      return lx;
    }
  func r()
  {
      return rx;
    }
  func norm()
  {
      if ((lineroot != -1))
      {
        lineroot = shiftfunc(lineroot, (t - tline));
        tline = t;
      }
    }
  func split(x: dynamic)
  {
      assert(((lx < x) && (x <= rx)));
      norm();
      var ret = Region(x, rx, blockcnt, -1, tline);
      rx = (x - 1);
      if ((lineroot != -1))
      {
        linetree.split(lineroot, (x - t), lineroot, ret.lineroot);
      }
      return ret;
    }
}

class SumRegion
{
  var mnblockcnt: dynamic;
  func SumRegion()
  {
      mnblockcnt = INT_MAX;
    }
  func SumRegion(region: dynamic)
  {
      mnblockcnt = if ((region.lineroot == -1)) INT_MAX else region.blockcnt;
    }
}

func operator_add_assign(a: dynamic, b: dynamic)
{
  a.mnblockcnt = min(a.mnblockcnt, b.mnblockcnt);
  return a;
}

class LazyRegion
{
  var lazyblockcnt: dynamic;
  func LazyRegion()
  {
      lazyblockcnt = 0;
    }
  func LazyRegion(lazyblockcnt: dynamic)
  {
      this->lazyblockcnt = cpp_construct(lazyblockcnt);
    }
}

func operator_add_assign(a: dynamic, b: dynamic)
{
  a.blockcnt += b.lazyblockcnt;
  return a;
}

func operator_add_assign(a: dynamic, b: dynamic)
{
  if ((a.mnblockcnt != INT_MAX))
  {
    a.mnblockcnt += b.lazyblockcnt;
  }
  return a;
}

func operator_add_assign(a: dynamic, b: dynamic)
{
  a.lazyblockcnt += b.lazyblockcnt;
  return a;
}

var regiontree: dynamic;

func print(regionroot: dynamic)
{
  var allregions = regiontree.all(regionroot);
  for (var region in allregions)
  {
    printf("[%d..%d] = %d:", region.lx, region.rx, region.blockcnt);
    if (((region.lineroot != -1) && (region.tline != t)))
    {
      printf(" (delay %d)", (t - region.tline));
    }
    printfunc(region.lineroot, region.tline);
  }
}

func killzeroes(node: dynamic)
{
  regiontree.splay(node);
  while ((regiontree.nodes[node].sum.mnblockcnt == 0))
  {
    while (((regiontree.nodes[node].item.blockcnt != 0) || (regiontree.nodes[node].item.lineroot == -1)))
    {
      if (((regiontree.nodes[node].ch[0] != -1) && (regiontree.nodes[regiontree.nodes[node].ch[0]].sum.mnblockcnt == 0)))
      {
        node = regiontree.nodes[node].ch[0];
      } else
      {
        node = regiontree.nodes[node].ch[1];
      }
      assert(((node != -1) && (regiontree.nodes[node].sum.mnblockcnt == 0)));
    }
    assert((regiontree.nodes[node].item.lineroot != -1));
    regiontree.splay(node);
    regiontree.nodes[node].item.lineroot = -1;
    regiontree.update(node);
  }
  return node;
}

func rgrow(node: dynamic, x: dynamic)
{
  var l: dynamic;
  var r: dynamic;
  regiontree.split(node, (x + 1), l, r);
  l = regiontree.last(l);
  r = regiontree.first(r);
  if (((l == -1) || (regiontree.nodes[l].item.lineroot == -1)))
  {
    return regiontree.join(l, r);
  }
  while ((((r != -1) && (regiontree.nodes[r].item.lineroot == -1)) && (regiontree.nodes[r].item.blockcnt == 0)))
  {
    assert(((regiontree.nodes[l].item.rx + 1) == regiontree.nodes[r].item.lx));
    regiontree.nodes[l].item.rx = regiontree.nodes[r].item.rx;
    r = regiontree.disconnect(r, 1);
    r = regiontree.first(r);
  }
  if (((r != -1) && (regiontree.nodes[r].item.lineroot != -1)))
  {
    assert(((regiontree.nodes[l].item.rx + 1) == regiontree.nodes[r].item.lx));
    regiontree.nodes[l].item.rx = regiontree.nodes[r].item.lx;
    regiontree.nodes[l].item.norm();
    regiontree.nodes[l].item.lineroot = rgrowfunc(regiontree.nodes[l].item.lineroot, (regiontree.nodes[l].item.rx - t));
    regiontree.nodes[r].item.norm();
    regiontree.nodes[l].item.lineroot = mergefunc(regiontree.nodes[l].item.lineroot, regiontree.nodes[r].item.lineroot);
    regiontree.nodes[l].item.rx = regiontree.nodes[r].item.rx;
    r = regiontree.disconnect(r, 1);
  } else
  {
    regiontree.nodes[l].item.norm();
    regiontree.nodes[l].item.lineroot = rgrowfunc(regiontree.nodes[l].item.lineroot, (regiontree.nodes[l].item.rx - t));
  }
  var ret = regiontree.join(l, r);
  return ret;
}

func lgrow(node: dynamic, x: dynamic)
{
  var l: dynamic;
  var r: dynamic;
  regiontree.split(node, x, l, r);
  l = regiontree.last(l);
  r = regiontree.first(r);
  if (((r == -1) || (regiontree.nodes[r].item.lineroot == -1)))
  {
    return regiontree.join(l, r);
  }
  while ((((l != -1) && (regiontree.nodes[l].item.lineroot == -1)) && (regiontree.nodes[l].item.blockcnt == 0)))
  {
    assert(((regiontree.nodes[l].item.rx + 1) == regiontree.nodes[r].item.lx));
    regiontree.nodes[r].item.lx = regiontree.nodes[l].item.lx;
    l = regiontree.disconnect(l, 0);
    l = regiontree.last(l);
  }
  if (((l != -1) && (regiontree.nodes[l].item.lineroot != -1)))
  {
    assert(((regiontree.nodes[l].item.rx + 1) == regiontree.nodes[r].item.lx));
    regiontree.nodes[l].item.rx = regiontree.nodes[r].item.lx;
    regiontree.nodes[l].item.norm();
    regiontree.nodes[l].item.lineroot = rgrowfunc(regiontree.nodes[l].item.lineroot, (regiontree.nodes[l].item.rx - t));
    regiontree.nodes[r].item.norm();
    regiontree.nodes[l].item.lineroot = mergefunc(regiontree.nodes[l].item.lineroot, regiontree.nodes[r].item.lineroot);
    regiontree.nodes[l].item.rx = regiontree.nodes[r].item.rx;
    r = regiontree.disconnect(r, 1);
  } else
  {
    regiontree.nodes[r].item.norm();
    regiontree.nodes[r].item.lineroot = lgrowfunc(regiontree.nodes[r].item.lineroot, (regiontree.nodes[r].item.lx - t));
  }
  return regiontree.join(l, r);
}

func solve()
{
  linetree.reset();
  regiontree.reset();
  var linedn = linetree.add(Line((-INF), sx, abs(((-INF) - sx)), -1));
  var lineup = linetree.add(Line(sx, (+INF), 0, +1));
  var lineroot = linetree.join(linedn, lineup);
  var regionroot = regiontree.add(Region((-INF), (+INF), 0, lineroot, 0));
  var e: dynamic;
  REP(i, nrect).PB(MP(((2 * rect[i].lt) + 1), i));
  e.PB(MP(((2 * rect[i].rt) + 0), i));
  sort(e.begin(), e.end());
  var finalregions = regiontree.all(regionroot);
  assert((SZ(finalregions) == 1));
  var finalregion = finalregions[0];
  finalregion.norm();
  assert((finalregion.lineroot != -1));
  var finalfunc = linetree.all(finalregion.lineroot);
  var ret = INT_MAX;
  for (var func_cpp in finalfunc)
  {
    ret = min(ret, min(func_cpp.lcost, func_cpp.rcost()));
  }
  return ret;
}

func run()
{
  scanf("%d", (&nrect));
  scanf("%d", (&sx));
  REP(i, nrect);
  scanf("%d%d%d%d", (&rect[i].lt), (&rect[i].rt), (&rect[i].lx), (&rect[i].rx));
  rect[i].lt -= 1;
  rect[i].rt += 1;
  rect[i].lx -= 1;
  rect[i].rx += 1;
  printf("%d\n", solve());
}

func stress()
{
  var mxrect = 100;
  var mxdim = 100;
  REP(rep, 10000);
  {
    nrect = ((rnd() % mxrect) + 1);
    var tdim = ((rnd() % mxdim) + 1);
    var xdim = ((rnd() % mxdim) + 1);
    sx = (rnd() % ((xdim + 1)));
    solve();
    printf(".");
  }
}

func main()
{
  run();
  return 0;
}

func REPSZ(argument_0: dynamic, argument_1: dynamic)
{
    t = (e[i].first >> 1);
    var kind = (e[i].first & 1);
    var idx = e[i].second;
    if ((kind == 0))
    {
      var l: dynamic;
      var m: dynamic;
      var r: dynamic;
      regiontree.split(regionroot, (rect[idx].lx + 1), l, m);
      regiontree.split(m, rect[idx].rx, m, r);
      regiontree.apply(m, LazyRegion(-1));
      regionroot = regiontree.join(regiontree.join(l, m), r);
      regionroot = rgrow(regionroot, rect[idx].lx);
      regionroot = lgrow(regionroot, rect[idx].rx);
    }
    if ((kind == 1))
    {
      var l: dynamic;
      var m: dynamic;
      var r: dynamic;
      regiontree.split(regionroot, (rect[idx].lx + 1), l, m);
      regiontree.split(m, rect[idx].rx, m, r);
      m = killzeroes(m);
      regiontree.apply(m, LazyRegion(1));
      regionroot = regiontree.join(regiontree.join(l, m), r);
    }
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      rect[i].lt = (rnd() % tdim);
      rect[i].rt = (rnd() % tdim);
      if ((rect[i].lt > rect[i].rt))
      {
        swap(rect[i].lt, rect[i].rt);
      }
      rect[i].rt += 2;
      rect[i].lx = (rnd() % xdim);
      rect[i].rx = (rnd() % xdim);
      if ((rect[i].lx > rect[i].rx))
      {
        swap(rect[i].lx, rect[i].rx);
      }
      rect[i].rx += 2;
    }
