// Translated from solution.cpp.

var kMax = 100010;

class Query
{
  var ql: dynamic;
  var qr: dynamic;
  var qi: dynamic;
  func Query(ql: dynamic, qr: dynamic, qi: dynamic)
  {
      ql = ql;
      qr = qr;
      qi = qi;
    }
  func operator_less(rq: dynamic)
  {
      return (qr < rq.qr);
    }
}

var N: dynamic;

var Q: dynamic;

var si = cpp_array(kMax);

var di = cpp_array(kMax);

var lf = cpp_array(kMax);

var rg = cpp_array(kMax);

var dfn: dynamic;

var max_dpt: dynamic;

var tree = cpp_array(kMax);

var vsi = cpp_array(kMax);

var vlf = cpp_array(kMax);

var vrg = cpp_array(kMax);

var qvi = cpp_array(kMax);

var qki = cpp_array(kMax);

var vq = cpp_array(kMax);

var res = cpp_array(kMax);

var f = cpp_array(kMax);

var rm_pos: dynamic;

func low_bit(i: dynamic)
{
  return (i & ((-i)));
}

func main()
{
  ios.sync_with_stdio(false);
  read();
  prep();
  soups_on();
  return 0;
}

func read(argument_0: dynamic)
{
  var str: dynamic;
  var prn: dynamic;
  var id_cnt: dynamic;
  var name_id: dynamic;
  read(N);
  id_cnt = 0;
  {
    var i = 1;
    while ((i <= N))
    {
      read(str, prn);
      if ((name_id.count(str) == 0))
      {
        name_id[str] = cpp_update(id_cnt, "++");
      }
      si[i] = name_id[str];
      tree[prn].push_back(i);
      i += 1;
    }
  }
  read(Q);
  {
    var i = 0;
    while ((i < Q))
    {
      read(qvi[i], qki[i]);
      i += 1;
    }
  }
}

func prep(argument_0: dynamic)
{
  var vi: dynamic;
  var ki: dynamic;
  var d: dynamic;
  var l: dynamic;
  var r: dynamic;
  dfs(0, 0);
  {
    var i = 0;
    while ((i < Q))
    {
      vi = qvi[i];
      ki = qki[i];
      d = (di[vi] + ki);
      if ((d <= max_dpt))
      {
        l = (lower_bound(vlf[d].begin(), vlf[d].end(), lf[vi]) - vlf[d].begin());
        r = (lower_bound(vrg[d].begin(), vrg[d].end(), rg[vi]) - vrg[d].begin());
        vq[d].push_back(Query(l, r, i));
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= max_dpt))
    {
      stable_sort(vq[i].begin(), vq[i].end());
      i += 1;
    }
  }
}

func soups_on(argument_0: dynamic)
{
  {
    var i = 1;
    while ((i <= max_dpt))
    {
      var p: dynamic;
      var q: dynamic;
      p = cpp_assign(q, "=", 0);
      while ((q < int_cpp(vq[i].size())))
      {
        var rq = vq[i][q];
        while ((p < rq.qr))
        {
          if (rm_pos.count(vsi[i][p]))
          {
            update((rm_pos[vsi[i][p]] + 1), -1);
          }
          rm_pos[vsi[i][p]] = p;
          update((p + 1), 1);
          p += 1;
        }
        res[rq.qi] = (query_sum(rq.qr) - query_sum(rq.ql));
        q += 1;
      }
      {
        var itr = rm_pos.begin();
        while ((itr != rm_pos.end()))
        {
          update((itr->second + 1), -1);
          itr += 1;
        }
      }
      rm_pos.clear();
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < Q))
    {
      write(res[i], "\n");
      i += 1;
    }
  }
}

func dfs(u: dynamic, lv: dynamic)
{
  di[u] = lv;
  vsi[lv].push_back(si[u]);
  vlf[lv].push_back(cpp_assign(lf[u], "=", dfn));
  dfn += 1;
  {
    var i = 0;
    while ((i < int_cpp(tree[u].size())))
    {
      dfs(tree[u][i], (lv + 1));
      i += 1;
    }
  }
  vrg[lv].push_back(cpp_assign(rg[u], "=", dfn));
  dfn += 1;
  max_dpt = max(max_dpt, lv);
}

func update(i: dynamic, dlt: dynamic)
{
  while ((i < kMax))
  {
    f[i] += dlt;
    i += low_bit(i);
  }
}

func query_sum(i: dynamic)
{
  var ret = 0;
  while ((i > 0))
  {
    ret += f[i];
    i -= low_bit(i);
  }
  return ret;
}
