// Translated from solution.cpp.

var MAX_N = 500;

var INF = (1 << 30);

var n: dynamic;

var m: dynamic;

var nst: dynamic;

var sstr: dynamic;

var pstr: dynamic;

var gstr: dynamic;

var si: dynamic;

var pi: dynamic;

var gi: dynamic;

var sids: dynamic;

var nbrs = cpp_array(MAX_N);

var dists = cpp_array(MAX_N);

func sid(str: dynamic)
{
  var mit = sids.find(str);
  if ((mit == sids.end()))
  {
    return (cpp_assign(sids[str], "=", cpp_update(nst, "++")));
  }
  return mit->second;
}

func mindist(st: dynamic, gl: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      dists[i] = INF;
      i += 1;
    }
  }
  dists[st] = 0;
  var q: dynamic;
  q.push(pii(0, st));
  while ((!q.empty()))
  {
    var u = q.top();
    q.pop();
    var ud = u.first;
    var ui = u.second;
    if ((ud != dists[ui]))
    {
      continue;
    }
    if ((ui == gl))
    {
      break;
    }
    var nbru = nbrs[ui];
    {
      var vit = nbru.begin();
      while ((vit != nbru.end()))
      {
        var vi = vit->first;
        var vd = (ud + vit->second);
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

func main()
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
        var i = 0;
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
        var i = 0;
        while ((i < m))
        {
          var astr: dynamic;
          var bstr: dynamic;
          var di: dynamic;
          var ti: dynamic;
          read(astr, bstr, di, ti);
          var ai = sid(astr);
          var bi = sid(bstr);
          var d = ((di / 40) + ti);
          nbrs[ai].push_back(pii(bi, d));
          nbrs[bi].push_back(pii(ai, d));
          i += 1;
        }
      }
      var mind = (mindist(si, pi) + mindist(pi, gi));
      write(mind, "\n");
    }
  }
  return 0;
}
