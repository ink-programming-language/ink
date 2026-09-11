// Translated from solution.cpp.

func toInt(s: dynamic) -> dynamic
{
  var v: dynamic = cpp_uninitialized();
  (sin >> v);
  return v;
}

func toString(x: dynamic) -> dynamic
{
  var sout: dynamic = cpp_uninitialized();
  (sout << x);
  return sout.str();
}

func ALL(a: dynamic) -> dynamic
{
  return cpp_expression("#include <vector> #in");
}

func RALL(a: dynamic) -> dynamic
{
  return cpp_expression("#include <vector> #incl");
}

func EXIST(s: dynamic, e: dynamic) -> dynamic
{
  return cpp_expression("#include <vector> #inclu");
}

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func REP(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

func EACH(t: dynamic, i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(t::iterator i=(c).begin(); i!=(c).end(); ++i)");
}

var EPS: dynamic = 1e-10;

var PI: dynamic = acos(-1.0);

func main() -> dynamic
{
  var m: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  while (cpp_comma((((cin >> m) >> c) >> n), m))
  {
    var q: dynamic = cpp_uninitialized();
    var cost: dynamic = 0;
    while ((!q.empty()))
    {
      var p: dynamic = q.front();
      q.pop();
      var req: dynamic = p.front();
      p.erase(p.begin(), (p.begin() + 1));
      var founddesk: dynamic = -1;
      var foundloc: dynamic = -1;
      if ((founddesk == -1))
      {
        cost += (m + 1);
      } else
      {
        d[founddesk].erase((d[founddesk].begin() + foundloc), ((d[founddesk].begin() + foundloc) + 1));
        cost += (founddesk + 1);
      }
      if ((d[0].size() < c))
      {
        d[0].push_back(req);
        cost += 1;
      } else
      {
        var put: dynamic = false;
        var tempdesk: dynamic = -1;
        var temploc: dynamic = -1;
        if ((!put))
        {
          tempdesk = -1;
          cost += (m + 1);
        }
        var lru: dynamic = d[0].front();
        d[0].erase(d[0].begin(), (d[0].begin() + 1));
        cost += 1;
        var putlru: dynamic = false;
        FOR(j, 1, m);
        {
          if ((d[j].size() < c))
          {
            d[j].push_back(lru);
            putlru = true;
            cost += (j + 1);
            break;
          }
        }
        if ((!putlru))
        {
          cost += (m + 1);
        }
        if ((tempdesk != -1))
        {
          d[tempdesk].erase((d[tempdesk].begin() + temploc), ((d[tempdesk].begin() + temploc) + 1));
          cost += (tempdesk + 1);
        } else
        {
          cost += (m + 1);
        }
        d[0].push_back(req);
        cost += 1;
      }
      if ((!p.empty()))
      {
        q.push(p);
      }
    }
    write(cost, "\n");
  }
}

func REP(argument_0: dynamic, i: dynamic) -> dynamic
{
        var bb: dynamic = cpp_uninitialized();
        read(bb);
        b[i].push_back(bb);
      }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      read(k[i]);
      q.push(b[i]);
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var it: dynamic = find(ALL(d[i]), req);
        if ((it != d[i].end()))
        {
          founddesk = i;
          foundloc = (it - d[i].begin());
          break;
        }
      }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
          if ((d[i].size() < c))
          {
            d[i].push_back(req);
            put = true;
            tempdesk = i;
            temploc = (d[i].size() - 1);
            cost += (i + 1);
            break;
          }
        }
