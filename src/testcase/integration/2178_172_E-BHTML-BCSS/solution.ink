// Translated from solution.cpp.

var fin: dynamic = cpp_construct("input.in");

var fout: dynamic = cpp_construct("output.out");

var S: dynamic = cpp_uninitialized();

var P: dynamic = cpp_array(1000005);

var str: dynamic = cpp_uninitialized();

var pind: dynamic = cpp_uninitialized();

func add(str: dynamic) -> dynamic
{
  if ((str[0] == cpp_char("/")))
  {
    str = str.substr(1);
    P[cpp_update(pind, "++")] = make_pair(str, 0);
  } else if ((str[(str.size() - 1)] == cpp_char("/")))
  {
    str.resize((str.size() - 1));
    P[cpp_update(pind, "++")] = make_pair(str, 1);
    P[cpp_update(pind, "++")] = make_pair(str, 0);
  } else
  {
    P[cpp_update(pind, "++")] = make_pair(str, 1);
  }
}

func parcala() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var temp: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < str.size()))
    {
      if ((str[i] == cpp_char(">")))
      {
        add(temp);
        temp = "";
      } else if ((str[i] != cpp_char("<")))
      {
        temp += str[i];
      }
      i += 1;
    }
  }
}

func solve() -> dynamic
{
  read(str);
  parcala();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var g: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  var res: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var Q: dynamic = cpp_array(205);
  var q: dynamic = cpp_uninitialized();
  var temp: dynamic = cpp_uninitialized();
  read(M);
  getline(cin, q);
  {
    i = 1;
    while ((i <= M))
    {
      getline(cin, q);
      fill(Q, (Q + 202), "");
      n = 0;
      {
        j = 0;
        while ((j <= q.size()))
        {
          if (((q[j] == cpp_char(" ")) || (j == q.size())))
          {
            Q[cpp_update(n, "++")] = temp;
            temp = "";
          } else
          {
            temp += q[j];
          }
          j += 1;
        }
      }
      g = 1;
      res = 0;
      {
        j = 1;
        while ((j <= pind))
        {
          if ((P[j].second == 1))
          {
            if (((g <= n) && (P[j].first == Q[g])))
            {
              S.push(1);
              g += 1;
            } else
            {
              S.push(0);
            }
            if (((g == (n + 1)) && (P[j].first == Q[n])))
            {
              res += 1;
            }
          } else
          {
            t = S.top();
            S.pop();
            if (t)
            {
              g -= 1;
            }
          }
          j += 1;
        }
      }
      write(res, "\n");
      i += 1;
    }
  }
}

func main() -> dynamic
{
  solve();
}
