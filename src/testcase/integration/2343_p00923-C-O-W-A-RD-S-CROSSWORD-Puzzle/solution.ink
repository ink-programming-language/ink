// Translated from solution.cpp.

func ALL(c: dynamic)
{
  return cpp_expression("// very kuso prob");
}

func IN(l: dynamic, v: dynamic, r: dynamic)
{
  return cpp_expression("// very kuso pr");
}

func UNIQUE(v: dynamic)
{
  sort(ALL(v));
  v.erase(unique(ALL(v)), v.end());
}

func DUMP(x: dynamic)
{
  return cpp_expression("// very kuso problem #in");
}

func LINE()
{
  return cpp_expression("// very kuso problem #include");
}

func range(i: dynamic, l: dynamic, r: dynamic)
{
  cpp_macro("for(int i=(int)l;i<(int)(r);i++)");
}

func operator_shift_right(is: dynamic, p: dynamic)
{
  return ((is >> p.first) >> p.second);
}

func operator_shift_right(is: dynamic, t: dynamic)
{
  return (is >> get(t));
}

func operator_shift_right(is: dynamic, t: dynamic)
{
  return ((is >> get(t)) >> get(t));
}

func operator_shift_right(is: dynamic, t: dynamic)
{
  return (((is >> get(t)) >> get(t)) >> get(t));
}

func operator_shift_right(is: dynamic, t: dynamic)
{
  return ((((is >> get(t)) >> get(t)) >> get(t)) >> get(t));
}

func operator_shift_right(is: dynamic, as_cpp: dynamic)
{
  range(i, 0, as_cpp.size());
  (is >> as_cpp[i]);
  return is;
}

func operator_shift_left(os: dynamic, ss: dynamic)
{
  for (var a in ss)
  {
    if ((a != ss.begin()))
    {
      (os << " ");
    }
    (os << a);
  }
  return os;
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  return (((os << p.first) << " ") << p.second);
}

func operator_shift_left(os: dynamic, m: dynamic)
{
  var isF = true;
  for (var p in m)
  {
    if ((!isF))
    {
      (os << endl);
    }
    (os << p);
    isF = false;
  }
  return os;
}

func operator_shift_left(os: dynamic, t: dynamic)
{
  return (os << get(t));
}

func operator_shift_left(os: dynamic, t: dynamic)
{
  return (((os << get(t)) << " ") << get(t));
}

func operator_shift_left(os: dynamic, t: dynamic)
{
  return (((((os << get(t)) << " ") << get(t)) << " ") << get(t));
}

func operator_shift_left(os: dynamic, t: dynamic)
{
  return (((((((os << get(t)) << " ") << get(t)) << " ") << get(t)) << " ") << get(t));
}

func operator_shift_left(os: dynamic, as_cpp: dynamic)
{
  range(i, 0, as_cpp.size());
  {
    if ((i != 0))
    {
      (os << " ");
    }
    (os << as_cpp[i]);
  }
  return os;
}

func operator_shift_left(os: dynamic, as_cpp: dynamic)
{
  range(i, 0, as_cpp.size());
  {
    if ((i != 0))
    {
      (os << endl);
    }
    (os << as_cpp[i]);
  }
  return os;
}

class Edge
{
  var c: dynamic;
  var f: dynamic;
  var t: dynamic;
}

class NFA
{
  var ONE: dynamic;
  var N: dynamic;
  var es: dynamic;
  var f_cache: dynamic;
  var t_cache: dynamic;
  var reachable: dynamic;
  var trans: dynamic;
  var s: dynamic;
  var t: dynamic;
  func NFA(N: dynamic = 0)
  {
      this->N = cpp_construct(N);
      this->s = cpp_construct(0);
      this->t = cpp_construct(1);
      f_cache = cpp_assign(t_cache, "=", vector(N));
    }
  func add_edge(c: dynamic, f: dynamic, t: dynamic)
  {
      var eid = es.size();
      es.push_back([c, f, t]);
      f_cache[f].push_back(eid);
      t_cache[t].push_back(eid);
    }
  func disjoint(a: dynamic, b: dynamic)
  {
      var res = (if ((a->es.size() < b->es.size())) b else a);
      var add = (if ((a->es.size() < b->es.size())) a else b);
      res->f_cache.resize((res->N + add->N));
      res->t_cache.resize((res->N + add->N));
      var eid = res->es.size();
      for (var e in add->es)
      {
        e.f += res->N;
        e.t += res->N;
        res->es.push_back(e);
        res->f_cache[e.f].push_back(eid);
        res->t_cache[e.t].push_back(eid);
        eid += 1;
      }
      res->N += add->N;
      return res;
    }
  func Union(a: dynamic, b: dynamic)
  {
      var bl = (a->es.size() < b->es.size());
      var as_cpp = ((if (bl) b->N else 0) + a->s);
      var at = ((if (bl) b->N else 0) + a->t);
      var bs = ((if ((!bl)) a->N else 0) + b->s);
      var bt = ((if ((!bl)) a->N else 0) + b->t);
      var S = (a->N + b->N);
      var T = ((a->N + b->N) + 1);
      var ab = disjoint(a, b);
      var ab1 = disjoint(ab, (&ONE));
      var res = disjoint(ab1, (&ONE));
      res->add_edge(cpp_char("-"), S, as_cpp);
      res->add_edge(cpp_char("-"), S, bs);
      res->add_edge(cpp_char("-"), at, T);
      res->add_edge(cpp_char("-"), bt, T);
      res->s = S;
      res->t = T;
      return res;
    }
  func Concat(a: dynamic, b: dynamic)
  {
      var bl = (a->es.size() < b->es.size());
      var as_cpp = ((if (bl) b->N else 0) + a->s);
      var at = ((if (bl) b->N else 0) + a->t);
      var bs = ((if ((!bl)) a->N else 0) + b->s);
      var bt = ((if ((!bl)) a->N else 0) + b->t);
      var res = disjoint(a, b);
      res->add_edge(cpp_char("-"), at, bs);
      res->s = as_cpp;
      res->t = bt;
      return res;
    }
  func Star(a: dynamic)
  {
      var res = disjoint(a, (&ONE));
      res->add_edge(cpp_char("-"), (res->N - 1), a->s);
      res->add_edge(cpp_char("-"), a->t, (res->N - 1));
      res->s = cpp_assign(res->t, "=", (res->N - 1));
      return res;
    }
  func create_reachable()
  {
      var que: dynamic;
      que.push([t, 0]);
      reachable = vector(N, vector(4));
      reachable[t][0] = true;
      while ((!que.empty()))
      {
        var t: dynamic;
        var d: dynamic;
        tie(t, d) = que.front();
        que.pop();
        for (var eid in t_cache[t])
        {
          var e = es[eid];
          if ((e.c == cpp_char("-")))
          {
            if ((!reachable[e.f][d]))
            {
              reachable[e.f][d] = true;
              que.push([e.f, d]);
            }
          } else
          {
            if ((((d + 1) < reachable[e.f].size()) && (!reachable[e.f][(d + 1)])))
            {
              reachable[e.f][(d + 1)] = true;
              que.push([e.f, (d + 1)]);
            }
          }
        }
      }
    }
  func create_trans()
  {
      trans = vector(N, vector(27));
      range(st, 0, N);
      {
        passed[st] = true;
        var que: dynamic;
        range(i, 0, passed.size());
        if (passed[i])
        {
          que.push(i);
        }
        while ((!que.empty()))
        {
          var s = que.front();
          que.pop();
          for (var eid in f_cache[s])
          {
            if (((es[eid].c == cpp_char("-")) && (!passed[es[eid].t])))
            {
              passed[es[eid].t] = true;
              que.push(es[eid].t);
            }
          }
        }
        range(i, 0, N);
        if (passed[i])
        {
          trans[st][26].push_back(i);
        }
      }
      range(st, 0, N);
      range(c, cpp_char("A"), (cpp_char("Z") + 1));
      {
        for (var s in trans[st][26])
        {
          passed[s] = true;
        }
        var que: dynamic;
        range(i, 0, passed.size());
        if (passed[i])
        {
          que.push(i);
        }
        while ((!que.empty()))
        {
          var s = que.front();
          que.pop();
          for (var eid in f_cache[s])
          {
            if (((((es[eid].c == c) || (es[eid].c == cpp_char(".")))) && (!npassed[es[eid].t])))
            {
              npassed[es[eid].t] = true;
              que.push(es[eid].t);
            }
          }
        }
        range(i, 0, N);
        if (npassed[i])
        {
          for (var s in trans[i][26])
          {
            npassed[s] = true;
          }
        }
        range(s, 0, npassed.size());
        if (npassed[s])
        {
          var alleps = true;
          for (var eid in f_cache[s])
          {
            alleps &= (es[eid].c == cpp_char("-"));
          }
          if ((alleps && (t != s)))
          {
            continue;
          }
          trans[st][(c - cpp_char("A"))].push_back(s);
        }
      }
    }
}

var ONE = NFA(1);

class REGtoNFA
{
  func pattern(s: dynamic, i: dynamic, e: dynamic)
  {
      var p = simple(s, i, e);
      while (((i < e) && (s[i] == cpp_char("|"))))
      {
        i += 1;
        var q = simple(s, i, e);
        p = NFA.Union(p, q);
      }
      return p;
    }
  func simple(s: dynamic, i: dynamic, e: dynamic)
  {
      var p = basic(s, i, e);
      while (((i < e) && (s[i] != cpp_char("|"))))
      {
        var q = basic(s, i, e);
        p = NFA.Concat(p, q);
      }
      return p;
    }
  func basic(s: dynamic, i: dynamic, e: dynamic)
  {
      var p = elementary(s, i, e);
      if (((i < cpp_cast(s.size())) && (s[i] == cpp_char("*"))))
      {
        i += 1;
        p = NFA.Star(p);
      }
      return p;
    }
  func elementary(s: dynamic, i: dynamic, e: dynamic)
  {
      var p: dynamic;
      if ((s[i] == cpp_char("(")))
      {
        i += 1;
        var d = 1;
        {
          var j = i;
          while ((j < e))
          {
            if ((s[j] == cpp_char("(")))
            {
              d += 1;
            }
            if ((s[j] == cpp_char(")")))
            {
              d -= 1;
            }
            if ((d == 0))
            {
              e = j;
              break;
            }
            j += 1;
          }
        }
        p = pattern(s, i, e);
        i += 1;
      } else
      {
        p = cpp_new(2);
        p->add_edge(s[i], p->s, p->t);
        i += 1;
      }
      return p;
    }
}

var rc = 0;

class Main
{
  var h: dynamic;
  var w: dynamic;
  var tmp: dynamic;
  var res: dynamic;
  var NFAs: dynamic;
  var stats: dynamic;
  func dfs(y: dynamic, x: dynamic, c: dynamic)
  {
      if ((rc > 1))
      {
        return;
      }
      if ((y == h))
      {
        rc += c;
        res = tmp;
        return;
      }
      var nstats = cpp_construct(2, vector(26));
      var nfai = [y, (h + x)];
      range(ni, 0, 2);
      {
        var nc = cpp_char("A");
        while ((nc <= cpp_char("Z")))
        {
          for (var s in stats[nfai[ni]])
          {
            for (var t in NFAs[nfai[ni]].trans[s][(nc - cpp_char("A"))])
            {
              nstats[ni][(nc - cpp_char("A"))].push_back(t);
            }
          }
          UNIQUE(nstats[ni][(nc - cpp_char("A"))]);
          nc += 1;
        }
      }
      var gid = cpp_construct(26);
      iota(ALL(gid), 0);
      {
        var c1 = cpp_char("A");
        while ((c1 <= cpp_char("Z")))
        {
          {
            var c2 = cpp_char("A");
            while ((c2 < c1))
            {
              if ((gid[(c2 - cpp_char("A"))] == (c2 - cpp_char("A"))))
              {
                if (((nstats[0][(c1 - cpp_char("A"))].size() == nstats[0][(c2 - cpp_char("A"))].size()) && (nstats[1][(c1 - cpp_char("A"))].size() == nstats[1][(c2 - cpp_char("A"))].size())))
                {
                  var eq = true;
                  range(ni, 0, 2);
                  {
                    range(i, 0, nstats[ni][(c1 - cpp_char("A"))].size());
                    {
                      eq &= (nstats[ni][(c1 - cpp_char("A"))][i] == nstats[ni][(c2 - cpp_char("A"))][i]);
                      if ((!eq))
                      {
                        break;
                      }
                    }
                    if ((!eq))
                    {
                      break;
                    }
                  }
                  if (eq)
                  {
                    gid[(c1 - cpp_char("A"))] = gid[(c2 - cpp_char("A"))];
                    break;
                  }
                }
              }
              c2 += 1;
            }
          }
          c1 += 1;
        }
      }
      var gc = cpp_construct(26);
      range(i, 0, 26)[gid[i]] += 1;
      {
        var c = cpp_char("A");
        while ((c <= cpp_char("Z")))
        {
          if ((gid[(c - cpp_char("A"))] == (c - cpp_char("A"))))
          {
            tmp[y][x] = c;
            var tmpOK = true;
            {
              var ok = false;
              for (var s in nstats[0][(c - cpp_char("A"))])
              {
                ok |= NFAs[y].reachable[s][((w - 1) - x)];
              }
              tmpOK &= ok;
            }
            {
              var ok = false;
              for (var s in nstats[1][(c - cpp_char("A"))])
              {
                ok |= NFAs[(h + x)].reachable[s][((h - 1) - y)];
              }
              tmpOK &= ok;
            }
            if ((x == (w - 1)))
            {
              var ok = false;
              for (var s in nstats[0][(c - cpp_char("A"))])
              {
                ok |= (s == NFAs[y].t);
              }
              tmpOK &= ok;
            }
            if ((y == (h - 1)))
            {
              var ok = false;
              for (var s in nstats[1][(c - cpp_char("A"))])
              {
                ok |= (s == NFAs[(h + x)].t);
              }
              tmpOK &= ok;
            }
            if (tmpOK)
            {
              var tmpp = stats[y];
              var tmpq = stats[(h + x)];
              stats[y] = nstats[0][(c - cpp_char("A"))];
              stats[(h + x)] = nstats[1][(c - cpp_char("A"))];
              if (((x + 1) < w))
              {
                dfs(y, (x + 1), gc[(c - cpp_char("A"))]);
              } else
              {
                dfs((y + 1), 0, gc[(c - cpp_char("A"))]);
              }
              stats[y] = tmpp;
              stats[(h + x)] = tmpq;
            }
          }
          c += 1;
        }
      }
    }
  func run()
  {
      while (true)
      {
        read(h, w);
        if (((h == 0) && (w == 0)))
        {
          break;
        }
        rc = 0;
        tmp = cpp_assign(res, "=", vector(h, string_cpp(w, cpp_char("-"))));
        var ss = cpp_construct((h + w));
        read(ss);
        NFAs = vector((h + w));
        range(i, 0, (h + w));
        {
          var s = "";
          range(j, 1, (ss[i].size() - 1)) += ss[i][j];
          var cur = 0;
          var e = s.size();
          NFAs[i] = (*REGtoNFA.pattern(s, cur, e));
        }
        range(i, 0, (h + w));
        {
          NFAs[i].create_reachable();
          NFAs[i].create_trans();
        }
        stats = vector((h + w));
        range(i, 0, (h + w))[i].push_back(NFAs[i].s);
        dfs(0, 0, 1);
        if ((rc > 1))
        {
          write("ambiguous", "\n");
        } else if ((rc == 0))
        {
          write("none", "\n");
        } else
        {
          ((range(i, 0, h) << res[i]) << endl);
        }
      }
    }
}

func main()
{
  write(fixed, setprecision(20));
  cin.tie(0);
  ios.sync_with_stdio(false);
  Main().run();
  return 0;
}
