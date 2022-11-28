import matplotlib.pyplot as plt

hof = open("./res/hist_origin_std.txt", "r")
for line in hof:
    original_hist = list(map(int, line.split()))
hef = open("./res/hist_equalized_std.txt", "r")
for line in hef:
    equalized_hist = list(map(int, line.split()))

plt.figure(0)
hist_x = [x for x in range(256)]
plt.xticks(range(0,255,25))
plt.bar(hist_x, original_hist, width=1)
plt.savefig("./res/std_hist.png")

plt.figure(1)
plt.bar(hist_x, equalized_hist, width=1)
plt.savefig("./res/std_hist_equalized.png")

