// Translated from solution.cpp.

var big: dynamic = (1e15 + 100000);

var mod: dynamic = (1e9 + 7);

var eps: dynamic = 1e-9;

var pai: dynamic = 3.141592653589793238462643;

var mt: dynamic = cpp_expression("#include<c");

var mp: dynamic = cpp_expression("#include<");

var fir: dynamic = cpp_expression("#incl");

var sec: dynamic = cpp_expression("#inclu");

var pub: dynamic = cpp_expression("#include<");

var puf: dynamic = cpp_expression("#include<c");

var pob: dynamic = cpp_expression("#include");

var pof: dynamic = cpp_expression("#include<");

var res: dynamic = cpp_expression("#inclu");

var ins: dynamic = cpp_expression("#inclu");

var era: dynamic = cpp_expression("#incl");

func dme(in_cpp: dynamic) -> dynamic
{
  cpp_macro("cout<<in<<endl;return 0");
}

func mineq(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func maxeq(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
  }
}

func main(argument_0: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  read(n, p);
  var dp: dynamic = [0];
  var dag: dynamic = cpp_uninitialized();
  var kra: dynamic = cpp_uninitialized();
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
      var s: dynamic = cpp_uninitialized();
      var e: dynamic = cpp_uninitialized();
      var t1: dynamic = cpp_uninitialized();
      var t2: dynamic = cpp_uninitialized();
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
    var ba: dynamic = kra.front();
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
      var no: dynamic = dag[i];
      {
        j = 0;
        while ((j < mti[no].size()))
        {
          var mae: dynamic = mti[no][j].fir;
          mineq(dp[no][no], (dp[mae][mae] + mti[no][j].sec.sec));
          var ti: dynamic = mti[no][j].sec.fir;
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
              var mak: dynamic = mti[no][k].fir;
              if ((mae == mak))
              {
                k += 1;
                continue;
              }
              var tk: dynamic = mti[no][k].sec.fir;
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
