// Translated from solution.cpp.

var MAX_N: dynamic = 500;

var INF: dynamic = (1 << 30);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var nst: dynamic = cpp_uninitialized();

var sstr: dynamic = cpp_uninitialized();

var pstr: dynamic = cpp_uninitialized();

var gstr: dynamic = cpp_uninitialized();

var si: dynamic = cpp_uninitialized();

var pi: dynamic = cpp_uninitialized();

var gi: dynamic = cpp_uninitialized();

var sids: dynamic = cpp_uninitialized();

var nbrs: dynamic = cpp_array(MAX_N);

var dists: dynamic = cpp_array(MAX_N);

func sid(str: dynamic) -> dynamic
{
  var mit: dynamic = sids.find(str);
  if ((mit == sids.end()))
  {
    return (cpp_assign(sids[str], "=", cpp_update(nst, "++")));
  }
  return mit->second;
}

func mindist(st: dynamic, gl: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      dists[i] = INF;
      i += 1;
    }
  }
  dists[st] = 0;
  var q: dynamic = cpp_uninitialized();
  q.push(pii(0, st));
  while ((!q.empty()))
  {
    var u: dynamic = q.top();
    q.pop();
    var ud: dynamic = u.first;
    var ui: dynamic = u.second;
    if ((ud != dists[ui]))
    {
      continue;
    }
    if ((ui == gl))
    {
      break;
    }
    var nbru: dynamic = nbrs[ui];
    {
      var vit: dynamic = nbru.begin();
      while ((vit != nbru.end()))
      {
        var vi: dynamic = vit->first;
        var vd: dynamic = (ud + vit->second);
        if ((dists[vi] > vd))
        {
          dists[vi] = vd;
          q.push(pii(vd, vi));
        }
        vit += 1;
      }
    }
  }
  return dists[gl];
}

func main() -> dynamic
{
  {
    while (true)
    {
      read(n, m);
      if ((n == 0))
      {
        break;
      }
      sids.clear();
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          nbrs[i].clear();
          i += 1;
        }
      }
      read(sstr, pstr, gstr);
      nst = 0;
      si = sid(sstr);
      pi = sid(pstr);
      gi = sid(gstr);
      {
        var i: dynamic = 0;
        while ((i < m))
        {
          var astr: dynamic = cpp_uninitialized();
          var bstr: dynamic = cpp_uninitialized();
          var di: dynamic = cpp_uninitialized();
          var ti: dynamic = cpp_uninitialized();
          read(astr, bstr, di, ti);
          var ai: dynamic = sid(astr);
          var bi: dynamic = sid(bstr);
          var d: dynamic = ((di / 40) + ti);
          nbrs[ai].push_back(pii(bi, d));
          nbrs[bi].push_back(pii(ai, d));
          i += 1;
        }
      }
      var mind: dynamic = (mindist(si, pi) + mindist(pi, gi));
      write(mind, "\n");
    }
  }
  return 0;
}
