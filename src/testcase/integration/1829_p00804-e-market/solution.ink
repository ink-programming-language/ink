// Translated from solution.cpp.

func REP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i = 0; i < (int)n; i++)");
}

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i = a; i < (int)b; i++)");
}

var pb: dynamic = cpp_expression("#include");

var mp: dynamic = cpp_expression("#include");

var INF: dynamic = (1 << 28);

var MOD: dynamic = 1000000007;

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, 1, 0, -1];

class order
{
  var name: dynamic = cpp_uninitialized();
  var com: dynamic = cpp_uninitialized();
  var price: dynamic = cpp_uninitialized();
  func operator_less(a: dynamic) -> dynamic
  {
      return (price < a.price);
    }
}

class comodity
{
  var min: dynamic = cpp_uninitialized();
  var max: dynamic = cpp_uninitialized();
  var sum: dynamic = cpp_uninitialized();
  var cnt: dynamic = cpp_uninitialized();
}

func addCom(m: dynamic, v: dynamic, name: dynamic) -> dynamic
{
  if ((!m.count(name)))
  {
    var c: dynamic = cpp_uninitialized();
    c.min = cpp_assign(c.max, "=", cpp_assign(c.sum, "=", v));
    c.cnt = 1;
    m[name] = c;
  } else
  {
    m[name].min = min(m[name].min, v);
    m[name].max = max(m[name].max, v);
    m[name].sum += v;
    m[name].cnt += 1;
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  while (cpp_comma((cin >> n), n))
  {
    var buy: dynamic = cpp_uninitialized();
    var sell: dynamic = cpp_uninitialized();
    var ppl: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    for (var i: dynamic in m)
    {
      write(i.first, cpp_char(" "), i.second.min, cpp_char(" "), (i.second.sum / i.second.cnt), cpp_char(" "), i.second.max, "\n");
    }
    write("--", "\n");
    for (var i: dynamic in ppl)
    {
      write(i.first, cpp_char(" "), i.second.first, cpp_char(" "), i.second.second, "\n");
    }
    write("----------", "\n");
  }
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var o: dynamic = cpp_uninitialized();
      var in_cpp: dynamic = cpp_uninitialized();
      read(o.name, in_cpp, o.com, o.price);
      if ((!ppl.count(o.name)))
      {
        ppl[o.name] = mp(0, 0);
      }
      var flg: dynamic = false;
      var v: dynamic = cpp_uninitialized();
      var pnt: dynamic = cpp_uninitialized();
      if ((in_cpp == "SELL"))
      {
        REP(j, buy.size());
        {
          if ((((o.com == buy[j].com) && (o.name != buy[j].name)) && (o.price <= buy[j].price)))
          {
            v = (((o.price + buy[j].price)) / 2);
            addCom(m, v, o.com);
            flg = true;
            pnt = j;
            break;
          }
        }
        if (flg)
        {
          ppl[buy[pnt].name].first += v;
          ppl[o.name].second += v;
          buy.erase((buy.begin() + pnt));
        } else
        {
          sell.pb(o);
          stable_sort(sell.begin(), sell.end());
        }
      } else if ((in_cpp == "BUY"))
      {
        REP(j, sell.size());
        {
          if ((((o.com == sell[j].com) && (o.name != sell[j].name)) && (o.price >= sell[j].price)))
          {
            v = (((o.price + sell[j].price)) / 2);
            addCom(m, v, o.com);
            flg = true;
            pnt = j;
            break;
          }
        }
        if (flg)
        {
          ppl[o.name].first += v;
          ppl[sell[pnt].name].second += v;
          sell.erase((sell.begin() + pnt));
        } else
        {
          buy.pb(o);
          stable_sort(buy.rbegin(), buy.rend());
        }
      }
    }
