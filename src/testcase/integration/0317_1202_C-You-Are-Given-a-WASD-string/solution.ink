// Translated from solution.cpp.

var maxs: dynamic = 200000;

var dbuf: dynamic = "DWAS";

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, 1, 0, -1];

var s: dynamic = cpp_array((maxs + 1));

var xv: dynamic = cpp_array((maxs + 1));

var yv: dynamic = cpp_array((maxs + 1));

var lprv: dynamic = cpp_array((maxs + 1));

var bprv: dynamic = cpp_array((maxs + 1));

var rprv: dynamic = cpp_array((maxs + 1));

var tprv: dynamic = cpp_array((maxs + 1));

var lnxt: dynamic = cpp_array((maxs + 1));

var bnxt: dynamic = cpp_array((maxs + 1));

var rnxt: dynamic = cpp_array((maxs + 1));

var tnxt: dynamic = cpp_array((maxs + 1));

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var T: dynamic = cpp_uninitialized();
  read(T);
  {
    var TN: dynamic = 0;
    while ((TN < T))
    {
      read(s);
      var n: dynamic = strlen(s);
      xv[0] = 0;
      yv[0] = 0;
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          var d: dynamic = (find(dbuf, (dbuf + 4), s[i]) - dbuf);
          xv[(i + 1)] = (xv[i] + dx[d]);
          yv[(i + 1)] = (yv[i] + dy[d]);
          i += 1;
        }
      }
      lprv[0] = cpp_assign(rprv[0], "=", xv[0]);
      bprv[0] = cpp_assign(tprv[0], "=", yv[0]);
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          lprv[i] = min(lprv[(i - 1)], xv[i]);
          bprv[i] = min(bprv[(i - 1)], yv[i]);
          rprv[i] = max(rprv[(i - 1)], xv[i]);
          tprv[i] = max(tprv[(i - 1)], yv[i]);
          i += 1;
        }
      }
      lnxt[n] = cpp_assign(rnxt[n], "=", xv[n]);
      bnxt[n] = cpp_assign(tnxt[n], "=", yv[n]);
      {
        var i: dynamic = (n - 1);
        while ((i >= 0))
        {
          lnxt[i] = min(lnxt[(i + 1)], xv[i]);
          bnxt[i] = min(bnxt[(i + 1)], yv[i]);
          rnxt[i] = max(rnxt[(i + 1)], xv[i]);
          tnxt[i] = max(tnxt[(i + 1)], yv[i]);
          i -= 1;
        }
      }
      var ans: dynamic = 0x7f7f7f7f7f7f7f7f;
      {
        var i: dynamic = 0;
        while ((i <= n))
        {
          {
            var d: dynamic = 0;
            while ((d < 4))
            {
              var w: dynamic = ((max(rprv[i], (rnxt[i] + dx[d])) - min(lprv[i], (lnxt[i] + dx[d]))) + 1);
              var h: dynamic = ((max(tprv[i], (tnxt[i] + dy[d])) - min(bprv[i], (bnxt[i] + dy[d]))) + 1);
              ans = min(ans, (cpp_cast(w) * h));
              d += 1;
            }
          }
          i += 1;
        }
      }
      write(ans, cpp_char("\n"));
      TN += 1;
    }
  }
  return 0;
}
