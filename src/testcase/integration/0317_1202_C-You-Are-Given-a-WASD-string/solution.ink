// Translated from solution.cpp.

var maxs = 200000;

var dbuf = "DWAS";

var dx = [1, 0, -1, 0];

var dy = [0, 1, 0, -1];

var s = cpp_array((maxs + 1));

var xv = cpp_array((maxs + 1));

var yv = cpp_array((maxs + 1));

var lprv = cpp_array((maxs + 1));

var bprv = cpp_array((maxs + 1));

var rprv = cpp_array((maxs + 1));

var tprv = cpp_array((maxs + 1));

var lnxt = cpp_array((maxs + 1));

var bnxt = cpp_array((maxs + 1));

var rnxt = cpp_array((maxs + 1));

var tnxt = cpp_array((maxs + 1));

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var T: dynamic;
  read(T);
  {
    var TN = 0;
    while ((TN < T))
    {
      read(s);
      var n = strlen(s);
      xv[0] = 0;
      yv[0] = 0;
      {
        var i = 0;
        while ((i < n))
        {
          var d = (find(dbuf, (dbuf + 4), s[i]) - dbuf);
          xv[(i + 1)] = (xv[i] + dx[d]);
          yv[(i + 1)] = (yv[i] + dy[d]);
          i += 1;
        }
      }
      lprv[0] = cpp_assign(rprv[0], "=", xv[0]);
      bprv[0] = cpp_assign(tprv[0], "=", yv[0]);
      {
        var i = 1;
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
        var i = (n - 1);
        while ((i >= 0))
        {
          lnxt[i] = min(lnxt[(i + 1)], xv[i]);
          bnxt[i] = min(bnxt[(i + 1)], yv[i]);
          rnxt[i] = max(rnxt[(i + 1)], xv[i]);
          tnxt[i] = max(tnxt[(i + 1)], yv[i]);
          i -= 1;
        }
      }
      var ans = 0x7f7f7f7f7f7f7f7f;
      {
        var i = 0;
        while ((i <= n))
        {
          {
            var d = 0;
            while ((d < 4))
            {
              var w = ((max(rprv[i], (rnxt[i] + dx[d])) - min(lprv[i], (lnxt[i] + dx[d]))) + 1);
              var h = ((max(tprv[i], (tnxt[i] + dy[d])) - min(bprv[i], (bnxt[i] + dy[d]))) + 1);
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
