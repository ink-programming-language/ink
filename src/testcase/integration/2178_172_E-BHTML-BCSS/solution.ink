// Translated from solution.cpp.

var fin = cpp_construct("input.in");

var fout = cpp_construct("output.out");

var S: dynamic;

var P = cpp_array(1000005);

var str: dynamic;

var pind: dynamic;

func add(str: dynamic)
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

func parcala()
{
  var i: dynamic;
  var temp: dynamic;
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

func solve()
{
  read(str);
  parcala();
  var i: dynamic;
  var j: dynamic;
  var g: dynamic;
  var M: dynamic;
  var res: dynamic;
  var n: dynamic;
  var t: dynamic;
  var Q = cpp_array(205);
  var q: dynamic;
  var temp: dynamic;
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

func main()
{
  solve();
}
