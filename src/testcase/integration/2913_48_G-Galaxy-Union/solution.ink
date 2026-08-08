// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var k: dynamic;

var debug = false;

var W: dynamic;

class root
{
  var id: dynamic;
  var edge: dynamic;
  var ss: dynamic;
  var subNodes: dynamic;
}

var mp = cpp_array(200005);

var rs: dynamic;

var v = cpp_array(200005);

var cpoint = cpp_array(200005);

var sumNode = cpp_array(200005);

var countNode = cpp_array(200005);

var dd = cpp_array(200005);

var toOtherFromFAther = cpp_array(200005);

var ans = cpp_array(200005);

var rr: dynamic;

func findCircle(i: dynamic, p: dynamic)
{
  if (v[i])
  {
    return i;
  }
  v[i] = 1;
  {
    var it = mp[i].begin();
    while ((it != mp[i].end()))
    {
      var w = it->first;
      if ((w == p))
      {
        it += 1;
        continue;
      }
      var t = findCircle(w, i);
      if ((t != -1))
      {
        cpoint[w] = 1;
        var r: dynamic;
        r.id = w;
        r.edge = it->second;
        rs.push_back(r);
        return if ((t == i)) -1 else t;
      }
      it += 1;
    }
  }
  return -1;
}

func dfs(i: dynamic, p: dynamic)
{
  rr->subNodes.push_back(i);
  sumNode[i] = 0;
  countNode[i] = 1;
  {
    var it = mp[i].begin();
    while ((it != mp[i].end()))
    {
      var w = it->first;
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

func dfs1(i: dynamic, p: dynamic)
{
  {
    var it = mp[i].begin();
    while ((it != mp[i].end()))
    {
      var w = it->first;
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

func calTree(i: dynamic)
{
  rr = (&rs[i]);
  dd[rr->id] = 0;
  dfs(rr->id, -1);
  toOtherFromFAther[rr->id] = 0;
  dfs1(rr->id, -1);
  {
    var it = rr->subNodes.begin();
    while ((it != rr->subNodes.end()))
    {
      ans[(*it)] = (sumNode[(*it)] + toOtherFromFAther[(*it)]);
      it += 1;
    }
  }
  rr->ss = ans[rr->id];
}

func calCircle()
{
  var cursum = 0;
  var cnt = 0;
  var cirlen = 0;
  var sum = 0;
  var q = 0;
  var ee = cpp_array((200005 + 200005));
  {
    var i = 0;
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
    var i = q;
    while ((i < m))
    {
      cursum += (((cirlen - ((ee[i] - ee[0])))) * countNode[rs[i].id]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      while (((2 * ((ee[q] - ee[i]))) <= cirlen))
      {
        cursum = ((cursum - (((cirlen - ((ee[q] - ee[i])))) * countNode[rs[(q % m)].id])) + (((ee[q] - ee[i])) * countNode[rs[(q % m)].id]));
        cnt += countNode[rs[(q % m)].id];
        q += 1;
      }
      {
        var it = rs[i].subNodes.begin();
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

func main()
{
  scanf("%d", (&n));
  var a: dynamic;
  var b: dynamic;
  var t: dynamic;
  {
    var i = 0;
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
    var i = 0;
    while ((i < m))
    {
      calTree(i);
      i += 1;
    }
  }
  calCircle();
  {
    var i = 0;
    while ((i < n))
    {
      printf("%I64d ", ans[i]);
      i += 1;
    }
  }
  return 0;
}
