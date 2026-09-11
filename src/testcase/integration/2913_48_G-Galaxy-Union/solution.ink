// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var debug: dynamic = false;

var W: dynamic = cpp_uninitialized();

class root
{
  var id: dynamic = cpp_uninitialized();
  var edge: dynamic = cpp_uninitialized();
  var ss: dynamic = cpp_uninitialized();
  var subNodes: dynamic = cpp_uninitialized();
}

var mp: dynamic = cpp_array(200005);

var rs: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(200005);

var cpoint: dynamic = cpp_array(200005);

var sumNode: dynamic = cpp_array(200005);

var countNode: dynamic = cpp_array(200005);

var dd: dynamic = cpp_array(200005);

var toOtherFromFAther: dynamic = cpp_array(200005);

var ans: dynamic = cpp_array(200005);

var rr: dynamic = cpp_uninitialized();

func findCircle(i: dynamic, p: dynamic) -> dynamic
{
  if (v[i])
  {
    return i;
  }
  v[i] = 1;
  {
    var it: dynamic = mp[i].begin();
    while ((it != mp[i].end()))
    {
      var w: dynamic = it->first;
      if ((w == p))
      {
        it += 1;
        continue;
      }
      var t: dynamic = findCircle(w, i);
      if ((t != -1))
      {
        cpoint[w] = 1;
        var r: dynamic = cpp_uninitialized();
        r.id = w;
        r.edge = it->second;
        rs.push_back(r);
        return  ((t == i)) ? -1 : t;
      }
      it += 1;
    }
  }
  return -1;
}

func dfs(i: dynamic, p: dynamic) -> dynamic
{
  rr->subNodes.push_back(i);
  sumNode[i] = 0;
  countNode[i] = 1;
  {
    var it: dynamic = mp[i].begin();
    while ((it != mp[i].end()))
    {
      var w: dynamic = it->first;
      if (((w == p) || cpoint[w]))
      {
        it += 1;
        continue;
      }
      dd[w] += (dd[i] + it->second);
      dfs(w, i);
      countNode[i] += countNode[w];
      sumNode[i] += (sumNode[w] + (it->second * countNode[w]));
      it += 1;
    }
  }
}

func dfs1(i: dynamic, p: dynamic) -> dynamic
{
  {
    var it: dynamic = mp[i].begin();
    while ((it != mp[i].end()))
    {
      var w: dynamic = it->first;
      if (((w == p) || cpoint[w]))
      {
        it += 1;
        continue;
      }
      toOtherFromFAther[w] = ((((toOtherFromFAther[i] + sumNode[i]) - sumNode[w]) - (it->second * countNode[w])) + (it->second * ((countNode[rr->id] - countNode[w]))));
      dfs1(w, i);
      it += 1;
    }
  }
}

func calTree(i: dynamic) -> dynamic
{
  rr = (&rs[i]);
  dd[rr->id] = 0;
  dfs(rr->id, -1);
  toOtherFromFAther[rr->id] = 0;
  dfs1(rr->id, -1);
  {
    var it: dynamic = rr->subNodes.begin();
    while ((it != rr->subNodes.end()))
    {
      ans[(*it)] = (sumNode[(*it)] + toOtherFromFAther[(*it)]);
      it += 1;
    }
  }
  rr->ss = ans[rr->id];
}

func calCircle() -> dynamic
{
  var cursum: dynamic = 0;
  var cnt: dynamic = 0;
  var cirlen: dynamic = 0;
  var sum: dynamic = 0;
  var q: dynamic = 0;
  var ee: dynamic = cpp_array((200005 + 200005));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      cirlen += rs[i].edge;
      sum += rs[i].ss;
      ee[((i + 1) + m)] = cpp_assign(ee[(i + 1)], "=", rs[i].edge);
      i += 1;
    }
  }
  ee[0] = 0;
  partial_sum(ee, (((ee + m) + m) + 1), ee);
  while (((2 * ((ee[q] - ee[0]))) <= cirlen))
  {
    cursum += (((ee[q] - ee[0])) * countNode[rs[q].id]);
    cnt += countNode[rs[q].id];
    q += 1;
  }
  {
    var i: dynamic = q;
    while ((i < m))
    {
      cursum += (((cirlen - ((ee[i] - ee[0])))) * countNode[rs[i].id]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      while (((2 * ((ee[q] - ee[i]))) <= cirlen))
      {
        cursum = ((cursum - (((cirlen - ((ee[q] - ee[i])))) * countNode[rs[(q % m)].id])) + (((ee[q] - ee[i])) * countNode[rs[(q % m)].id]));
        cnt += countNode[rs[(q % m)].id];
        q += 1;
      }
      {
        var it: dynamic = rs[i].subNodes.begin();
        while ((it != rs[i].subNodes.end()))
        {
          ans[(*it)] += (((sum - rs[i].ss) + cursum) + (dd[(*it)] * ((n - countNode[rs[i].id]))));
          it += 1;
        }
      }
      cnt -= countNode[rs[i].id];
      cursum += (((n - cnt)) * rs[i].edge);
      cursum -= (cnt * rs[i].edge);
      i += 1;
    }
  }
}

func main() -> dynamic
{
  scanf("%d", (&n));
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d%d%d", (&a), (&b), (&t));
      a -= 1;
      b -= 1;
      mp[a].push_back(make_pair(b, t));
      mp[b].push_back(make_pair(a, t));
      i += 1;
    }
  }
  findCircle(0, -1);
  m = cpp_cast(rs.size());
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      calTree(i);
      i += 1;
    }
  }
  calCircle();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      printf("%I64d ", ans[i]);
      i += 1;
    }
  }
  return 0;
}
