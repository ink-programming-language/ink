// Translated from solution.cpp.

var big = (1e15 + 100000);

var mod = (1e9 + 7);

var eps = 1e-9;

var pai = 3.141592653589793238462643;

var mt = cpp_expression("#include<c");

var mp = cpp_expression("#include<");

var fir = cpp_expression("#incl");

var sec = cpp_expression("#inclu");

var pub = cpp_expression("#include<");

var puf = cpp_expression("#include<c");

var pob = cpp_expression("#include");

var pof = cpp_expression("#include<");

var res = cpp_expression("#inclu");

var ins = cpp_expression("#inclu");

var era = cpp_expression("#incl");

func dme(in_cpp: dynamic)
{
  cpp_macro("cout<<in<<endl;return 0");
}

func mineq(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func maxeq(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
  }
}

func main(argument_0: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var n: dynamic;
  var p: dynamic;
  read(n, p);
  var dp = [0];
  var dag: dynamic;
  var kra: dynamic;
  kra.push((n - 1));
  {
    i = 0;
    while ((i < n))
    {
      {
        j = 0;
        while ((j < n))
        {
          dp[i][j] = big;
          j += 1;
        }
      }
      i += 1;
    }
  }
  dp[0][0] = 0;
  {
    i = 0;
    while ((i < p))
    {
      var s: dynamic;
      var e: dynamic;
      var t1: dynamic;
      var t2: dynamic;
      read(s, e, t1, t2);
      s -= 1;
      e -= 1;
      zi[s] += 1;
      mti[e].pub(mp(s, mp(t1, (t1 + t2))));
      i += 1;
    }
  }
  while ((!kra.empty()))
  {
    var ba = kra.front();
    kra.pop();
    dag.pub(ba);
    {
      i = 0;
      while ((i < mti[ba].size()))
      {
        zi[mti[ba][i].fir] -= 1;
        if ((zi[mti[ba][i].fir] == 0))
        {
          kra.push(mti[ba][i].fir);
        }
        i += 1;
      }
    }
  }
  reverse(dag.begin(), dag.end());
  {
    i = 1;
    while ((i < dag.size()))
    {
      var no = dag[i];
      {
        j = 0;
        while ((j < mti[no].size()))
        {
          var mae = mti[no][j].fir;
          mineq(dp[no][no], (dp[mae][mae] + mti[no][j].sec.sec));
          var ti = mti[no][j].sec.fir;
          {
            k = 0;
            while ((k < i))
            {
              mineq(dp[dag[k]][no], (dp[dag[k]][mae] + ti));
              mineq(dp[no][dag[k]], (dp[dag[k]][mae] + ti));
              k += 1;
            }
          }
          {
            k = 0;
            while ((k < mti[no].size()))
            {
              var mak = mti[no][k].fir;
              if ((mae == mak))
              {
                k += 1;
                continue;
              }
              var tk = mti[no][k].sec.fir;
              mineq(dp[no][no], ((dp[mae][mak] + ti) + tk));
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(dp[(n - 1)][(n - 1)], "\n");
  return 0;
}
